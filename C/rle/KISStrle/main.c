#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include "IO.h"
#include "KISStrle.h"
#include "Threading.h"

/* Definitions of global variables declared in Threading.h and IO.h */
/* For details of what the different varialbes are, see the corresponding .h file */
/* IO.h */
char *buf;

uint16_t oy = 0;
uint16_t oz = 0;

/* Threading.h */
pthread_mutex_t init_thread_compression_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t update_global_state_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t thickness_passed_cond = PTHREAD_COND_INITIALIZER;

uint32_t finished_lines = 0;

/* Algorithm description */
/* KISStrle stands for Keep it simple, stupid threaded run-length encoding */
/* 
	For a description of the RLE algorithm, see rle.c in the parent folder.
	This is another attempt at multi-threading RLE. It works by having 4 consumer threads, and no producer thread.
	Each thread declares it's own buffer and reads lines from stdin independently of one another, updating global
	variables oy (offset y) and oz (offset z) under a mutex so that each thread can know where in the map it's lines
	are.

	It also has an extra feature that dynamically determines the number of lines buffered.
	The number of lines buffered by each thread is a quarter of a z layer, with a cap of 100MB per thread, meaning 
	a total max memory usage of approximately 400MB.

	There is a global variable, finished_lines, that determines the amount of lines compressed in the current parent
	thickness. This variable is updated by each thread under a mutex and is used to determine when the parent thickness
	has been compressed
*/
int main(void)
{
	pthread_t consumers[4]; 

	parseMetadata(); /* Parse metadata into global variables*/

	// Create and fork 4 consumer threads
	for (uint8_t i = 0; i < 4; i++) {
		pthread_create(&(consumers[i]), NULL, KISStrle, NULL);
	}	

	// Join the threads
	// Join doesn't return until the thread returns
	for (uint8_t i = 0; i < 4; i++) {
		pthread_join(consumers[i], NULL);
	}

	// Clean up
	pthread_mutex_destroy(&init_thread_compression_mutex);
	pthread_mutex_destroy(&update_global_state_mutex);
	pthread_cond_destroy(&thickness_passed_cond);

	return 0;
}