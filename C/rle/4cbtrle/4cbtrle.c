#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "IO.h"
#include "Threading.h"

// #define DEBUG 
/**
 * @brief The thread entry point function for all consumer threads
 * This function continuously reads and compresses all of the data in it's 
 * own part of the gloabl buf variable.
 * 
 * @param tid the made up thread id that is passed from main (ranges from 0 to num_threads)
 * and is used to determine which part of the buffer and which thread arguments each thread
 * should read from
 * @return void* unused, always returns NULL
 */
void* cbtrle(uint8_t tid)
{
	char *curr_line = buf + (tid * mx * 4); /* beginning of the threads private buffer */

	uint16_t thread_local_ox = 0; /* current x offset in the z-layer */
	uint16_t thread_local_oy = 0; /* current y offset in the z-layer */
	uint16_t itr_x = 0; // x iterator inside the current parent block

	char curr_tag; /* current tag in the RLE algorithm*/

	uint16_t run_start; // the position inside the current parent block where the run starts

	while (1) {

		// wait for the thread's buffer to be filled
		// there is undefined behaviour if the mutex that is used
		// in pthread_cond_wait is not locked before hand
		// such behaviour includes not returning from the wait
		pthread_mutex_lock(&consumer_status_check_mutex);
		while (!eof && !thread_params[tid].buf_filled) {
			#ifdef DEBUG 
			fprintf(stderr, "waiting on buffer %hhu to be filled\n", tid);
			#endif
			pthread_cond_wait(consumer_staus_check_conds + tid, &consumer_status_check_mutex);
			#ifdef DEBUG
			fprintf(stderr, "thread %hhu has been woken from wait\n", tid);
			#endif
		}
		pthread_mutex_unlock(&consumer_status_check_mutex);

		// If our buffer is in fact filled
		// The other case here is if the end of file has been reached
		// by the producer thread, in which case the thread should simply
		// exit
		if (thread_params[tid].buf_filled) {
			// run rle
			while (thread_local_oy < thread_params[tid].size_y) {
				while (thread_local_ox < mx) {
					curr_tag = curr_line[thread_local_ox + itr_x];  // save the compressed tag

					run_start = itr_x;			// sets the run_start to the current position in the x line
					itr_x++;

					// iterate until the tag is different or we are at the end of the parent block
					while ( ( itr_x % px ) && ( curr_line[thread_local_ox + itr_x] == curr_tag ) )
					{
						itr_x++;
					}

					output(
						thread_local_ox + run_start,
						thread_local_oy + thread_params[tid].given_y,
						thread_params[tid].given_z,
						itr_x - run_start,
						1,
						1,
						curr_tag);

					/* If we are at the end of a parent block, reset itr_x
					and increment thread_local_ox by the parent block size in the x direction */
					// if (itr_x % px == 0)
					// {
					// 	itr_x = 0;
					// }

					// th branchless version of the above code
					itr_x = (uint16_t) ( itr_x * (itr_x % px != 0) );
					thread_local_ox += (uint16_t) ( px * (itr_x % px == 0) );

					#ifdef DEBUG
					if (more_than_one_buf_filled_at_parent_block_thickness) fprintf(stderr, "Running with more_than_one_buf_filled_at_parent_block_thickness\n");
					#endif
				}

				// reset all relevant variables for moving to the next y line
				thread_local_ox = 0;
				thread_local_oy++;
				curr_line += mx;
			}

			// reset all variable for reading a new buffer
			thread_params[tid].buf_filled = 0;
			thread_local_oy = 0;
			curr_line = buf + (tid * mx * 4);

			#ifdef DEBUG
			fprintf(stderr, "buffer %hhu has been emptied\n", tid);
			#endif

			// let the producer thread know that there is an empty buffer
			sem_post(&empty_sem);
			pthread_cond_signal(&empty_cond);

			#ifdef DEBUG
			if (more_than_one_buf_filled_at_parent_block_thickness) fprintf(stderr, "posted and signalled that a buffer was emptied\n");
			#endif
		}
		else if (eof) { /* if (!thread_params[id].buf_filled) */
			return NULL;
		}
	}
}