#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//mode for init function
#define NET_SERVER 0
#define NET_CLIENT 1
//
#define NET_IPV4 4
#define NET_IPV6 6

//error codes
#define NET_OK    0 
#define NET_FAIL -1

#define NET_ERR_INIT_MODE 1
#define NET_ERR_INIT_ADDR 2
#define NET_ERR_INIT_SOCK 3
#define NET_ERR_INIT_BIND 4



#include "vec/vec.h"

union s_addr
{
	struct sockaddr_in6 v6;
	struct sockaddr_in  v4;
};

typedef struct
{
	fd_set read;
	fd_set write;
	int max;
}
fd_select;

struct net
{
	//socket file descriptor 
	int fd;
	//mode NET_CLIENT or NET_SERVER (i.e. bind or not)
	int mode;
	int version; //NET_IPV4 or NET_IPV6  i.e. 4 or 6


	vec_void_t   data;

	//addresses structure
	union s_addr addr;
	socklen_t    addr_len;

	union s_addr * current;
	socklen_t      current_len;

	struct timeval * timeout;

//	fd_select select;
};

//print errors and exit the program
void net_error ( int err );

//init
int net_init   ( struct net * restrict net, const short port, const char * restrict ip, int mode, int version );
//lazy init
int net_client ( struct net * restrict net, const short port, const char * restrict ip, int version );
int net_server ( struct net * restrict net, const short port, const char * restrict ip, int version );


int net_shutdown ( struct net * net );

//sendto and recvfrom with "struct net" structure
ssize_t net_write ( struct net * restrict net, const void * restrict buf, size_t len, int flags );
int net_read  ( struct net * restrict net,       void * restrict buf, size_t len, int flags );

int net_read2 ( struct net * net1, struct net * net2, void * buf, size_t len, int flags );


void net_set_timeout ( struct net * net, struct timeval * t );

#endif
