#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "IO.h"

void* zbtrle(void* arg)
{
	// Allocates buffer to buf map line into
	ThreadArg args = *((ThreadArg*) arg);

	char* curr_line = buf + ((mx + 1) * args.y_start);

	uint16_t thread_local_ox = 0; /* current x offset in the z-layer*/
	uint16_t thread_local_oy = 0; /* current y offset in the z-layer */
	uint16_t itr_x = 0; // x iterator inside the current parent block

	char curr_tag;

	uint16_t run_start = 0; // the position inside the current parent block where the run starts

	while (thread_local_oy < args.y_length) // while the block base is in bounds for the x-value
	{
		while (thread_local_ox < mx)
		{
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
				args.y_start + thread_local_oy,
				oz - 1,
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

		// Reset the thread_local_ox for moving on to the next y-line 
		thread_local_ox = 0;
		thread_local_oy++;

		curr_line += mx + 1;
	}

	return NULL;
}