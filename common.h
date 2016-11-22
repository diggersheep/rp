
#ifndef COMMON_H
#define COMMON_H

#define CHUNK_SIZE 1000 * 1000

#include "vec/vec.h"

typedef struct {
	char* filename;
	unsigned char* digest;
	vec_void_t chunkDigests;
} HashData;

#endif

