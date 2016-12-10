
#ifndef TRACKER_H
#define TRACKER_H

#include <sys/socket.h>

typedef struct {
	unsigned char hash[32];
	struct sockaddr client;
	int time_to_live;
} RegisteredHash;

#endif

