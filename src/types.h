#ifndef TYPES_H
#define TYPES_H

/*
Core type definitions that I like to use.
*/

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t b32;

typedef struct u8_array
{
	u8 *data;
	size_t size;
} u8_array;

#endif

