#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include "IO.h"
#include "trle.h"
#include "Threading.h"

char* buf;
sem_t empty_sem;
uint8_t eof = 0;

pthread_cond_t consumer_staus_check_conds[4] = {
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER,
	PTHREAD_COND_INITIALIZER
};

pthread_mutex_t consumer_status_check_mutexes[4] = { 
	PTHREAD_MUTEX_INITIALIZER, 
	PTHREAD_MUTEX_INITIALIZER, 
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER
};

ThreadParams thread_params[4] = {
	{ .buf_filled = 0 },
	{ .buf_filled = 0 },
	{ .buf_filled = 0 },
	{ .buf_filled = 0 }
};

/* Algorithm description */
/* zbtrle stands for z-layer buffered threaded run-length encoding*/
/* 
		- Compresses a parent block at a time
		- Moves left to right, then down one: (where x is the parent block)
		x0 -> 0x -> 00 -> 00
		00    00	x0	  0x
		- code for z commented out, waiting for implementation in parseBlock()
		- For every y, starts at an x value, saves it in RLE_base_x, saves the character
		at that position in curr_char and then iterates until the tag is different 
		or until we reach the end of the parent block.
		Whether we are at the end of the parent block or not is 
		determined by whether itr_x < parent_x.
		- Once block is finished, reset itr_x and itr_y and update x_base and y_base
		to point to the new parent block. If the block is out of bounds, finish
*/
int main(void)
{
	sem_init(&empty_sem, 0, 4);
	pthread_t consumers[4];

	uint16_t oy = 0;
	uint16_t oz = 0;

	parseMetadata();

	// Initialise buffer for loadBuf() to store data in.
	// Make it big enough to store 4 lines (including a null character at the end, to make fread happy), one for each thread
	buf = (char*) malloc(sizeof(char) * (mx + 1) * 4);

	for (uint8_t i = 0; i < 4; i++) {
		pthread_create(&(consumers[i]), NULL, trle, (void*) i);
	}	

	// Create a new thread for each z layer buffered, make it compress the first half of the
	// z layer, then compress the second half on the main thread
	// once both are done, join the thread, load a new buffer and create a new thread.
	// Not optimal performance. Could use the same thread repeatedly instead of creating a new
	// one each time
	while (!feof(stdin)) {
		for (uint8_t i = 0; i < 4; i++) {
			if (!thread_params[i].buf_filled) {

				if (oy == my) {
					oy = 0;
					oz++;
					scanf("\n\r");

					// if ( ( oz % mz) == 0) 
					if ( !( oz % mz ) ) {
						sem_wait(&empty_sem);
						sem_wait(&empty_sem);
						sem_wait(&empty_sem);
					}
				}

				fread(buf + ( i * mx ), 1, mx, stdin);
				scanf("\n\r");

				thread_params[i].buf_filled = 1;
				thread_params[i].given_y = oy;
				thread_params[i].given_z = oz;

				oy++;

				pthread_cond_signal(consumer_staus_check_conds + i);

				break;
			}
		}

		sem_wait(&empty_sem);
	}

	eof = 1;

	for (uint8_t i = 0; i < 4; i++) {
		pthread_cond_signal(consumer_staus_check_conds + i);
		pthread_join(consumers[i], NULL);
		pthread_mutex_destroy(consumer_status_check_mutexes + i);
		pthread_cond_destroy(consumer_staus_check_conds + i);
	}

	free(buf);
	sem_destroy(&empty_sem);

	return 0;
}