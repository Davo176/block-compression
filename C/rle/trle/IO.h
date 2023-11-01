#pragma once

#include <inttypes.h>
#include <pthread.h>

typedef struct
{
	char tag;
	char label[50];
} Tag;

// Used for passing arguments to threads
// explained in main.c
// typedef struct {
// 	char* buf_ptr;
// 	uint16_t y_start;
// 	uint16_t z;
// } ThreadArg;

#if  __WIN32__
	#define ENDLINE 2
#else
	#define ENDLINE 1
#endif

extern uint16_t mx;
extern uint16_t px;

extern uint16_t oz;

extern uint16_t mx, my, mz; // map x, y and z

extern char* buf;

extern char nada[ENDLINE]; // bin variable

// extern pthread_mutex_t thread_done_mutex;

// extern pthread_cond_t z_compressed_cond;

void parseMetadata();
int loadBuf(char* buf);
void output(const uint16_t ox, const uint16_t oy, const uint16_t oz, const uint16_t sx, const uint16_t sy, const uint16_t sz, char tag);