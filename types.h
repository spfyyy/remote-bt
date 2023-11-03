#ifndef TYPES_H
#define TYPES_H

/*
Core type definitions that I like to use.
*/

#include <stdint.h>

typedef struct u8_array
{
	uint8_t *data;
	size_t size;
} u8_array;

#endif
