#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

// #define DEBUG

#include "IO.h"
#include "4cbtrle.h"
#include "Threading.h"

/* actually declare all external variables noted in IO.h and Threading.h */
/* see the respective files for more information */
/* IO.h */
char *buf;
uint16_t oy = 0;
uint16_t oz = 0;

/* Threading.h */
sem_t empty_sem;
uint8_t eof = 0;

pthread_cond_t consumer_staus_check_conds[4] = {
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER
};

pthread_cond_t empty_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t empty_cond_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t consumer_status_check_mutex = PTHREAD_MUTEX_INITIALIZER;

ThreadParams thread_params[4] = {
	{ .buf_filled = 0 },
	{ .buf_filled = 0 },
	{ .buf_filled = 0 },
	{ .buf_filled = 0 }
};

#ifdef DEBUG
uint8_t more_than_one_buf_filled_at_parent_block_thickness = 0;
#endif

/* Algorithm description */
/* 4cbtrle stands for 4-line chunk buffered threaded run-length encoding*/
/* 
	for a description of the RLE algorithm, see rle.c in the parent folder.
	How the multithreading works in 4cbtrle:
	There is a producer and 4 consumer threads. The 4 in the name 4cbtrle does not stand
	for the number of threads, but how many lines each thread receives at a time.
	
	The producer thread (in this case in this file, main.c), initialises a buffer that
	is big enough to fit 4 lines for each of the 4 threads (16 lines in total), into which
	it reads lines from the input stream. It also initialises 4 ThreadArg structures, one 
	for each thread, which specify information about the data passed to the thread for compression
	so that the consumer thread can output it's compression directly.

	Perhaps ThreadArg is not a great name for the struct as Arg might indicate that the structure is a static argument
	that is passed only once at the forking of the thread, but in fact a ThreadArg struct is used as an inter-thread
	communication buffer between the producer and the consumer.

	The way that a consumer thread knows where to read in the buf and ThreadArgs structures (both of which are gloabl),
	is that when the threads are forked they are sent with a makeshift thread id (tid) that ranges from 0 to 3.
	This tid is then used as an offset from the global ThreadArgs structures and buffer respectively to read their own
	data.
	An example of such information is the given y and given z. This specifies which y and z values 
	in the input stream the lines passed to the thread correspond to.

	Once a producer has finished filling a buffer, it sends a "check status" signal to the corresponding
	consumer thread (there are 4 seperate pthread condition variables that do this for each of the threads).
	The corresponding consumer thread should then wake up, and check the status of it's buffer. buf_filled is
	another variable present in the ThreadArg struct that is continuously updated throughout the program.
	A semaphore is also used to keep track of how many buffers are currently empty (empty_bufs).

	Consumers wait on a "check_status" symbol from the producer thread that tells them to wake up and check if their
	buffer is full and they should compress their buffer or the producer thread reached the end of the file and they 
	should exit.

	When a consumer has finished compressing their buffer, they send an "empty" message through the "empty_cond" condition
	variable, which is waited on by the producer. When the producer picks up this signal, it iterates through the ThreadArgs
	structure to find a buffer that is not filled (via the buf_filled property which is set to 0 when the consumer is 
	done compressing)
*/
int main(void)
{
	sem_init(&empty_sem, 0, 4);
	pthread_t consumers[4];

	uint8_t thread_size_y;

	parseMetadata();

	// Initialise buffer
	// Make it big enough to store 4 lines for each of 4 consumer threads (don't need to include a null character at the end, fread has no notion of strings), one for each thread
	buf = (char *) malloc(sizeof(char) * mx * 4 * 4);

	// Create and fork threads, passing i as the tid (cast to a void*, this is a compile time warning but since
	// i is always less than 255, it's ok, I think)
	for (uint8_t i = 0; i < 4; i++) {
		pthread_create(&(consumers[i]), NULL, cbtrle, (void*) i);
	}	

	// Producer thread code
	// While the procducer thread is not at the end of the file
	while (!feof(stdin)) {
		// Iterate through all of the buffers to check if any of them are
		// not full
		// note that i in this case will end up being the thread id
		for (uint8_t i = 0; i < 4; i++) { 
			if (!thread_params[i].buf_filled) { // If a buffer is not full

				// Calculate how much to give to the thread
				// By default, give the thread 4 lines,
				// if there is less than 4 lines left to compress for this z layer,
				// give the thread the remaining number of lines
				if (my - oy >= 4) {
					thread_size_y = 4;
				}
				else {
					thread_size_y = my - oy;
				}

				// Lock the mutex so no consumer threads try to read their 
				// thread params during update of the global structure
				pthread_mutex_lock(&consumer_status_check_mutex);
				// read lines from stdin into corresponding (thread i) buffer
				for (uint8_t j = 0; j < thread_size_y; j++) {
					fread(buf + ( i * mx * 4 ) + ( j * mx ), 1, mx, stdin);
					scanf("\n"); // skip over new lines
				}

				// store information about the newly read information
				// in the corresponding thread's ThreadArg structure
				thread_params[i].buf_filled = 1;
				thread_params[i].given_y = oy;
				thread_params[i].given_z = oz;
				thread_params[i].size_y = thread_size_y;

				#ifdef DEBUG
				fprintf(stderr, "filled buf %hhu\n", i);
				#endif

				// Signal the thread to wake up and check if it's buffer is full
				pthread_cond_signal(consumer_staus_check_conds + i);
				pthread_mutex_unlock(&consumer_status_check_mutex);

				#ifdef DEBUG
				fprintf(stderr, "signalled that buf %hhu is filled\n", i);
				#endif

				// update where we currently are in the map
				oy += thread_size_y;

				if (oy == my) {
					oy = 0;
					oz++;
					scanf("\n");

					// if we are at a parent block thickness, wait for all of the threads
					// to complete their compression, and then start reading more data into
					// buffers after that
					// if ( ( oz % pz) == 0) 
					if ( !( oz % pz ) ) {
						#ifdef DEBUG
						fprintf(stderr, "About to wait for all buffers to empty for oz = %hu\n", oz);
						#endif

						#ifdef DEBUG
						uint8_t free_counter = 0;
						for(uint8_t j = 0; j < 4; j++) {
							free_counter += thread_params[j].buf_filled;
						}
						fprintf(stderr, "%hhu filled buffers\n", free_counter);

						if (free_counter > 0) {
							more_than_one_buf_filled_at_parent_block_thickness = 1;
							fprintf(stderr, "Set more_than_one_buf_filled_at_parent_block_thickness\n");
						}

						int sem_val;
						sem_getvalue(&empty_sem, &sem_val);
						fprintf(stderr, "sem val = %d\n", sem_val);
						#endif

						// wait for the 4 threads to complete
						while (sem_trywait(&empty_sem) != 0) {
							pthread_cond_wait(&empty_cond, &empty_cond_mutex);
						}

						#ifdef DEBUG
						fprintf(stderr, "Successfully waited on one thread\n");
						#endif

						while (sem_trywait(&empty_sem) != 0) {
							pthread_cond_wait(&empty_cond, &empty_cond_mutex);
						}

						#ifdef DEBUG
						fprintf(stderr, "Successfully waited on two threads\n");
						#endif

						while (sem_trywait(&empty_sem) != 0) {
							pthread_cond_wait(&empty_cond, &empty_cond_mutex);
						}

						#ifdef DEBUG
						fprintf(stderr, "Successfully waited on three threads\n");
						#endif

						while (sem_trywait(&empty_sem) != 0) {
							pthread_cond_wait(&empty_cond, &empty_cond_mutex);

							#ifdef DEBUG
							fprintf(stderr, "About to test the value of empty sem again\n");
							#endif
						}

						#ifdef DEBUG
						fprintf(stderr, "Successfully waited on all threads\n");
						#endif

						// At this point, the semaphore will be at 0 because it waited on all threads to be
						// complete, restore the semaphore to 4
						sem_post(&empty_sem);
						sem_post(&empty_sem);
						sem_post(&empty_sem);
						sem_post(&empty_sem);

						#ifdef DEBUG
						fprintf(stderr, "Waited on all buffers to empty and successfully posted semaphore back to 4 for oz = %hu\n", oz);

						more_than_one_buf_filled_at_parent_block_thickness = 0;
						#endif
					}
				}

				break;
			}
		}

		// wait for a consumer to signal that a buffer is empty
		while (sem_trywait(&empty_sem) != 0) {
			pthread_cond_wait(&empty_cond, &empty_cond_mutex);
		}
	}

	// set the global eof (end of file) flag and signal to all threads to check status
	// also clean up allocated memory
	eof = 1;

	for (uint8_t i = 0; i < 4; i++) {
		pthread_cond_signal(consumer_staus_check_conds + i);
		pthread_join(consumers[i], NULL);
		pthread_cond_destroy(consumer_staus_check_conds + i);
	}

	pthread_mutex_destroy(&consumer_status_check_mutex);

	free(buf);
	sem_destroy(&empty_sem);

	return 0;
}