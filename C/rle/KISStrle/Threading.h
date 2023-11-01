#pragma once

#include <pthread.h>
#include <inttypes.h>

extern pthread_mutex_t init_thread_compression_mutex; /* mutex used for intialising a thread's new compression block */
extern pthread_mutex_t update_global_state_mutex; /* mutex used for updating the global variables (i.e. oy and oz )*/

extern pthread_cond_t thickness_passed_cond; /* condition variable used to signal the current parent thickness is complete or wait on the rest of the current parent thickness to be compressed */

extern uint32_t finished_lines; /* the amount of lines compressed in the current parent thickness */
