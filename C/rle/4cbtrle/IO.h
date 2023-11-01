#pragma once

#include <inttypes.h>
#include <pthread.h>

typedef struct
{
	char tag;
	char label[50];
} Tag;

#if  __WIN32__
	#define ENDLINE 2
#else
	#define ENDLINE 1
#endif

/* GLOBAL METADATA VARIABLES*/
extern uint16_t mx, my, mz; // map x, y and z
extern uint8_t px, pz; /* parent x and y*/

extern uint16_t oz, oy; /* current offset in the y and z direction*/

extern char *buf; /* global buffer */

void parseMetadata();
void output(const uint16_t ox, const uint16_t oy, const uint16_t oz, const uint16_t sx, const uint16_t sy, const uint16_t sz, char tag);