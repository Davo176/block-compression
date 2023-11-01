#pragma once

#include <semaphore.h>
#include <pthread.h>
#include <inttypes.h>

typedef struct {
	uint16_t given_y;
	uint16_t given_z;
	uint8_t buf_filled;
} ThreadParams;

extern uint8_t eof;

extern sem_t empty_sem;

// extern pthread_cond_t consumer_1_buf_full;
// extern pthread_cond_t consumer_2_buf_full;
// extern pthread_cond_t consumer_3_buf_full;
// extern pthread_cond_t consumer_4_buf_full;
extern pthread_cond_t consumer_staus_check_conds[4];

// pthread_cond_t consumer_buf_filled_conds[4] = {
// 	PTHREAD_COND_INITIALIZER,
// 	PTHREAD_COND_INITIALIZER,
// 	PTHREAD_COND_INITIALIZER,
// 	PTHREAD_COND_INITIALIZER
// };

// extern pthread_mutex_t consumer_1_buf_mutex;
// extern pthread_mutex_t consuemr_2_buf_mutex;
// extern pthread_mutex_t consumer_3_buf_mutex;
// extern pthread_mutex_t consumer_4_buf_mutex;
extern pthread_mutex_t consumer_status_check_mutexes[4];

// pthread_mutex_t consumer_buf_mutexes[4] = { 
// 	PTHREAD_MUTEX_INITIALIZER, 
// 	PTHREAD_MUTEX_INITIALIZER, 
// 	PTHREAD_MUTEX_INITIALIZER,
// 	PTHREAD_MUTEX_INITIALIZER
// };

extern ThreadParams thread_params[4];