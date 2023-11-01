#pragma once

#include <inttypes.h>
#include <pthread.h>

/* typedef for Tag struct */
typedef struct
{
	char tag;
	char label[50];
} Tag;

extern uint16_t mx, my, mz; // map x, y and z

extern uint8_t px, pz; /* parent x and z */

extern uint16_t oz, oy; /* current y and z position */

extern uint16_t buf_y_sz; /* the amount of y lines to buffer per thread */

void parseMetadata();
void output(const uint16_t ox, const uint16_t oy, const uint16_t oz, const uint16_t sx, const uint16_t sy, const uint16_t sz, char tag);