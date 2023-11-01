#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "IO.h"
#include "Threading.h"

void* trle(uint8_t id)
{
	char* curr_line = buf + (id * mx);

	uint16_t thread_local_ox = 0; /* current x offset in the z-layer*/
	uint16_t itr_x = 0; // x iterator inside the current parent block

	char curr_tag;

	uint16_t run_start; // the position inside the current parent block where the run starts

	while (1) {

		while (!eof && !thread_params[id].buf_filled)
			pthread_cond_wait(consumer_staus_check_conds + id, consumer_status_check_mutexes + id);

		if (thread_params[id].buf_filled) {
			while (thread_local_ox < mx) {
				curr_tag = curr_line[thread_local_ox + itr_x];  // save the compressed tag

				run_start = itr_x;				// sets the run_start to the current position in the x line
				itr_x++;

				// iterate until the tag is different or we are at the end of the parent block
				while ( ( itr_x % px ) && ( curr_line[thread_local_ox + itr_x] == curr_tag ) )
				{
					itr_x++;
				}

				output(
					thread_local_ox + run_start,
					thread_params[id].given_y,
					thread_params[id].given_z,
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

				itr_x = (uint16_t) ( itr_x * (itr_x % px != 0) );
				thread_local_ox += (uint16_t) ( px * (itr_x % px == 0) );
			}

			thread_params[id].buf_filled = 0;
			thread_local_ox = 0;
			sem_post(&empty_sem);
		}
		else if (eof) { /* if (!thread_params[id].buf_filled) */
			return NULL;
		}
	}

	return NULL;
}