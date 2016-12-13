
#ifndef CLIENT_H
#define CLIENT_H

#include "hash.h"

#include <time.h>
#include <linux/limits.h>


/**/
typedef struct {
	HashData* hash_data;     /* structure to keep (or write) hash of file and chunks */
	char filename[PATH_MAX]; /* file name of the real file */
	enum {
		STATUS_PUT,
		STATUS_KEEP_ALIVE,
		STATUS_GET,
		STATUS_GET_CLIENT,
		STATUS_LIST
	} status; /* enum of all possibles states (useful with timeout) */
	int timeout; /* timeout in seconde for (re)send request/handle */

	vec_void_t related_clients;    /* SegmentClient */
	vec_void_t received_fragments; /* of int[1000]  */
} RegisteredFile;

#endif

