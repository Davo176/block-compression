#pragma once

#include <semaphore.h>
#include <pthread.h>
#include <inttypes.h>

// ThreadArgs structure typedef
typedef struct {
	uint16_t given_y;
	uint16_t given_z;
	uint8_t buf_filled;
	uint8_t size_y;
} ThreadParams;

extern uint8_t eof; /* end of file indicator */

extern sem_t empty_sem; /* amount of empty buffers */

extern pthread_cond_t consumer_staus_check_conds[4]; /* condition variables used to signal a status check from producer to consumer*/

extern pthread_mutex_t consumer_status_check_mutex; /* mutex used to seal the status check condition variables and the updating of the global ThreadArgs*/

extern pthread_cond_t empty_cond; /* condition variable to signal an empty buffer from consumer to producer */

extern pthread_mutex_t empty_cond_mutex; /* used to seal empty_cond */

extern ThreadParams thread_params[4]; /* Global ThreadArgs structures*/