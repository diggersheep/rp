
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define CHUNK_SIZE 1000 * 1000

#include "vec/vec.h"

typedef struct {
	char* filename;
	unsigned char* digest;
	vec_void_t chunkDigests;
} HashData;

typedef enum {
	REQUEST_GET,
	REQUEST_GET_ANSWER,
	REQUEST_LIST,
	REQUEST_LIST_ANSWER,

	/* Debug/orz */
	REQUEST_PRINT
} RequestType;

typedef struct {
	unsigned char type;
	unsigned char file_hash[32];
	unsigned char chunk_hash[32];
} RequestGet;

typedef struct {
	unsigned char type;
	unsigned file_hash[32];
} RequestList;

typedef struct {
	unsigned char type;
	unsigned char file_hash[32];
	uint16_t chunks_count;
	unsigned char chunk_hashes[32][0];
} RequestListAnswer;

#endif

