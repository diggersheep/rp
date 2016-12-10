
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

typedef struct __attribute__((__packed__)) {
	uint8_t  c;    /* arbitrary constant */
	uint16_t ipv;  /* version of IP address */
	uint16_t port;
	uint32_t address;
} SegmentClient4;

typedef struct __attribute__((__packed__)) {
	uint8_t  c;    /* arbitrary constant */
	uint16_t ipv;  /* version of IP address */
	uint16_t port;
	uint32_t address[4];
} SegmentClient6;

typedef union {
	SegmentClient4 v4;
	SegmentClient6 v6;
} SegmentClient;

typedef struct __attribute__((__packed__)) {
	uint8_t  c;
	uint16_t size;
	uint8_t hash[32];
} SegmentFileHash;

typedef struct __attribute__((__packed__)) {
	uint8_t  c;
	uint16_t size;
	uint8_t hash[32];
	uint16_t index;
} SegmentChunkHash;

typedef enum {
	REQUEST_LIST = 102,
	REQUEST_LIST_ANSWER = 103,

	REQUEST_PUT = 110,
	REQUEST_PUT_ACK = 111,
	REQUEST_GET = 112,
	REQUEST_GET_ACK = 113,

	/* Debug/orz */
	REQUEST_PRINT = 150,
	REQUEST_EC = 42
} RequestType;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	unsigned file_hash[32];
} RequestList;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	uint8_t file_hash[32];
	uint16_t chunks_count;
	uint8_t chunk_hashes[32][0];
} RequestListAnswer;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	SegmentFileHash hash_segment;
	SegmentClient client_segment;
} RequestPut;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	SegmentFileHash hash_segment;
	SegmentClient client_segment;
} RequestPutAck;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	SegmentFileHash hash_segment;
	SegmentClient client_segment;
} RequestGet;

typedef struct __attribute__((__packed__)) {
	uint8_t type;
	SegmentFileHash hash_segment;
	uint16_t count; /* Number of client segments to expect. */
	SegmentClient clients[0];
} RequestGetAck;

typedef struct __attribute__((__packed__)) {
	uint8_t  type;     /* packet type */
	uint8_t  subtype;  /* error or control indicator */
	uint16_t size;     /* size of raw data */
	uint8_t  data[0];  /* variable-length raw data. */
} RequestEC; /* Errors and Control */

#endif

