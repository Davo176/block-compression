#include <pthread.h>

#include <stdio.h> // only needed for debugging
#include <stdlib.h>

#include "IO.h"
#include "zbtrle.h"

char* buf;

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
	pthread_t t1;

	parseMetadata();

	/* RLE start */

	// Initialise buffer for loadBuf() to store data in.
	// Make it big enough to store a whole mx * my block (including '\n''s at the end of lines)
	buf = (char*) malloc(sizeof(char) * (mx + 1) * my);

	// Initialise the arguments to be sent to the threads
	// arguments specify where the thread should start reading and
	// how much it should read
	// Each thread is responsible for half of the current buffer
	ThreadArg t1_arg;
	ThreadArg t2_arg;

	t1_arg.y_length = my / 2;
	t1_arg.y_start = 0;

	t2_arg.y_length = (my / 2) + my % 2;
	t2_arg.y_start = my / 2;

	// Create a new thread for each z layer buffered, make it compress the first half of the
	// z layer, then compress the second half on the main thread
	// once both are done, join the thread, load a new buffer and create a new thread.
	// Not optimal performance. Could use the same thread repeatedly instead of creating a new
	// one each time
	while (loadBuf(buf))
	{
		pthread_create(&t1, NULL, zbtrle, (void*) &t1_arg);
		zbtrle((void*) &t2_arg );

		pthread_join(t1, NULL);
	}

	return 0;
}