#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "IO.h"
#include "Threading.h"

// #define DEBUG

void* KISStrle()
{
	#ifdef DEBUG
	long long unsigned int debug_tid = pthread_self();
	uint8_t debug_original_thickness;

	fprintf(stderr, "The thread debug tid is %llu\n", debug_tid);
	#endif

	// allocate the buffer, setting the current line to the start of the buffer
	char *buf = malloc(sizeof(char) * buf_y_sz * mx);
	char *curr_line = buf;

	uint16_t thread_local_ox = 0; /* current x offset in the z-layer*/
	uint16_t thread_local_finished_y = 0; /* current amount of y lines finished, used to determine if the thread is finished it's allocated block */
	uint16_t thread_local_oy; /* y value of the first line in the thread's buffer*/
	uint16_t thread_local_oz; /* z value of the first line in the thread's buffer, each thread cannot buffer over a z layer*/
	uint16_t itr_x = 0; // x iterator inside the current parent block

	uint32_t thread_local_size_y; /* amount of lines to buffer */

	char curr_tag; /* current tag in the RLE algorithm */

	uint16_t run_start; // the position inside the current parent block where the run starts

	// while we aren't at the end of the map
	while (!feof(stdin)) {

		pthread_mutex_lock(&init_thread_compression_mutex);
		// if we are at the end of a z layer
		if ( oy == my ) {

			#ifdef DEBUG
			debug_original_thickness = oz / pz;
			#endif

			// if we are at the end of a parent z thickness
			if ( ( (oz + 1) % pz) == 0 ) {

				#ifdef DEBUG
				fprintf(stderr, "Thickness %hhu: ", debug_original_thickness);
				if ( finished_lines < my ) {
					fprintf(stderr, "finished_lines < my\n");
				}
				else if (finished_lines == my) {
					fprintf(stderr, "finished_lines == my\n");
				}
				#endif

				// wait for the rest of the threads to finish compressing before moving to the next
				// parent thickness
				while ( (oz % pz) != 0 ) {
					#ifdef DEBUG
					fprintf(stderr, "Thread %llu waiting for rest of thickness %d to be compressed\n", debug_tid, oz / pz);
					#endif
					pthread_cond_wait(&thickness_passed_cond, &init_thread_compression_mutex);
					#ifdef DEBUG
					fprintf(stderr, "Thread %llu woke up from it's sleep\n", debug_tid);
					#endif
				}

				#ifdef DEBUG
				fprintf(stderr, "Thread %llu noticed that rest of thickness %d has been compressed, original thickness: %d\n", debug_tid, ( oz / pz ) - 1, debug_original_thickness);
				#endif
			} else { // If we are at the end of a z layer, but it isn't the end of a parent z thickness
				#ifdef DEBUG
				fprintf(stderr, "Thread %llu noticed a new z layer, current finished_lines = %hu\n", debug_tid, finished_lines);
				#endif
				// move to the next z layer
				oz++;
				oy = 0;
			}
		}

		// save where the lines in the buffer that are about to be read in are in the map
		thread_local_oy = oy;
		thread_local_oz = oz;

		// Determine how many lines to read into the threads buffer
		// Threads cannot read over z layers
		if ( ( my - oy ) >= buf_y_sz) {
			thread_local_size_y = buf_y_sz;
		}
		else {
			thread_local_size_y = my - oy;
		}

		// read into buffer
		for (uint8_t i = 0; i < thread_local_size_y; i++) {
			fread(curr_line + mx * i, 1, mx, stdin);
			scanf("\n");
		}

		// ignore a new line at the end of the z layer
		if ( ( my - oy ) <= 4) {
			scanf("\n");
		}

		// increment the global current y position by the amount of lines just buffered in
		// to this thread's buffer
		oy += thread_local_size_y;

		#ifdef DEBUG
		fprintf(stderr, "Thread %llu obtained thread_local_oy %hu, thread_local_size_y %hu, and thread_local_oz %hu\n", debug_tid, thread_local_oy, thread_local_size_y, thread_local_oz);
		#endif
		pthread_mutex_unlock(&init_thread_compression_mutex);

		// run rle
		while (thread_local_finished_y < thread_local_size_y) {
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
					thread_local_oy,
					thread_local_oz,
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

			thread_local_ox = 0;
			thread_local_oy++;
			thread_local_finished_y++; /* update the amount of lines finished */
			curr_line += mx; /* move to the next line in the buffer */
		}

		curr_line = buf; /* reset the curr_line to the start of this threads buffer*/
		thread_local_finished_y = 0;

		pthread_mutex_lock(&update_global_state_mutex);
		finished_lines += thread_local_size_y; /* update the global amount of finished lines */
		
		#ifdef DEBUG
		fprintf(stderr, "Thread %llu finished compressing up to line %hu of oz %hu, finished_lines = %hu\n", debug_tid, thread_local_oy, thread_local_oz, finished_lines); // note that here, thread_local_oy is at it's maximum becuase it's finished compressing
		#endif

		// if we are at the end of a parent block thickness and all of the lines in this parent block thickness have
		// been compressed
		if ( ( !( ( oz + 1 ) % pz ) ) && finished_lines == ( my * pz )) {
			#ifdef DEBUG
			fprintf(stderr, "Thread %llu signalling compressed parent thickness %d with finished_lines = %hu\n", debug_tid, oz / pz, finished_lines);
			#endif

			// move to the next z layer
			// note that this will only be run in one thread
			oz++;
			oy = 0;
			finished_lines = 0;
			pthread_cond_broadcast(&thickness_passed_cond); /* signal to any waiting threads that they can begin work on the next parent block thickness*/
		}
		pthread_mutex_unlock(&update_global_state_mutex);
	}

	free(buf);

	return NULL;
}