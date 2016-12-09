
#ifndef TRACKER_H
#define TRACKER_H

#include <sys/socket.h>

typedef struct {
	unsigned char hash[32];
	struct sockaddr client;
} RegisteredHash;

#endif

