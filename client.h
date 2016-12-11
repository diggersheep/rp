
#ifndef CLIENT_H
#define CLIENT_H

#include "hash.h"

#include <time.h>
#include <linux/limits.h>

typedef struct {
	HashData* hash_data;
	char filename[PATH_MAX];
	enum {
		STATUS_PUT,
		STATUS_KEEP_ALIVE,
		STATUS_GET,
		STATUS_LIST
	} status;
	int timeout;

	vec_void_t related_clients;
	vec_void_t received_fragments; /* of int[1000] */
} RegisteredFile;

#endif

