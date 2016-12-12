#include "net.h"
#include "vec/vec.h"

#include "debug.h"

//print errors and exit
void
net_error ( int err )
{
	switch ( err )
	{
		case NET_ERR_INIT_MODE:
			fprintf( stderr, "  [ERR] - net_init() - mode not allowed.\n");
			break;
		case NET_ERR_INIT_ADDR:
			fprintf( stderr, "  [ERR] - net_init() - invalid IP address.\n");
			break;
		case NET_ERR_INIT_SOCK:
			fprintf( stderr, "  [ERR] - net_init() - connot create a socket.\n");
			break;
		case NET_ERR_INIT_BIND:
			fprintf( stderr, "  [ERR] - net_init() - binding error.\n");
			break;
		default: //no error
			return;
	}
	exit( err );
}


//init struct net
int
net_init ( struct net * restrict net, const short port, const char * restrict ip, int mode, int version )
{
	int err = 0; // error return

	// check mode
	if ( mode != NET_SERVER && mode != NET_CLIENT)
		return NET_ERR_INIT_MODE;

	//init struct
	net->addr_len   = sizeof(net->addr);
	net->mode       = mode;
	net->fd         = -1;
	net->version    = version;


	net->current_len = 0;
	net->current     = NULL;
	net->timeout     = NULL;
	
	//set in memory sockaddr_in6 + init sockaddr_in6
	if (version == NET_IPV6 )
	{
		net->addr_len = sizeof(struct sockaddr_in6);
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v6.sin6_family = AF_INET6;
		net->addr.v6.sin6_port   = htons(port);
		err = inet_pton( AF_INET6, ip, &(net->addr.v6.sin6_addr) );
	}
	else // set in memory sockaddr_in + init socckaddr_in
	{
		net->addr_len    = sizeof(struct sockaddr_in);
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v4.sin_family = AF_INET;
		net->addr.v4.sin_port   = htons(port);
		err = inet_pton( AF_INET , ip, &(net->addr.v4.sin_addr));
	}

	// err if @ip can't be copying
	if ( err != 1 )
		return NET_ERR_INIT_ADDR;
	printf("Init on address %s and %d port.\n", ip, port);

	//init socket UDP - ipV6
	if (version == NET_IPV6)
		net->fd = socket(AF_INET6, SOCK_DGRAM, 0);
	else
		net->fd = socket(AF_INET , SOCK_DGRAM, 0);

	if ( net->fd == -1 )
		return NET_ERR_INIT_SOCK;

	// bind if it's a server
	if ( mode == NET_SERVER )
	{
		err = bind(
			net->fd,
			(struct sockaddr *) &(net->addr),
			net->addr_len
		);

		if ( err < 0 )
			return NET_ERR_INIT_BIND;
		vec_init( &(net->data) );
	}

	return NET_OK;
}




int
net_init_raw ( struct net * restrict net, const short port, const char * restrict ip, int mode, int version )
{
	int err = 0; // error return

	// check mode
	if ( mode != NET_SERVER && mode != NET_CLIENT)
		return NET_ERR_INIT_MODE;

	//init struct
	net->addr_len   = sizeof(net->addr);
	net->mode       = mode;
	net->fd         = -1;
	net->version    = version;


	net->current_len = 0;
	net->current     = NULL;
	net->timeout     = NULL;
	
	//set in memory sockaddr_in6 + init sockaddr_in6
	if (version == NET_IPV6 )
	{
		net->addr_len = sizeof(struct sockaddr_in6);
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v6.sin6_family = AF_INET6;
		net->addr.v6.sin6_port   = port;
		
		memcpy(
			&(net->addr.v6.sin6_addr),
			ip,
			sizeof(net->addr.v6.sin6_addr)
		);
	}
	else // set in memory sockaddr_in + init socckaddr_in
	{
		net->addr_len    = sizeof(struct sockaddr_in);
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v4.sin_family = AF_INET;
		net->addr.v4.sin_port   = port;
		memcpy(
			&(net->addr.v4.sin_addr),
			ip,
			sizeof(net->addr.v4.sin_addr)
		);
	}


	if (1) {
		char address[64];
		inet_ntop(version == NET_IPV4 ? AF_INET : AF_INET6, ip, address, sizeof(address));
		// err if @ip can't be copying
		printf("Init on address %s and %d port.\n", address, ntohs(port));
	}

	//init socket UDP - ipV6
	if (version == NET_IPV6)
		net->fd = socket(AF_INET6, SOCK_DGRAM, 0);
	else
		net->fd = socket(AF_INET , SOCK_DGRAM, 0);

	if ( net->fd == -1 )
		return NET_ERR_INIT_SOCK;

	// bind if it's a server
	if ( mode == NET_SERVER )
	{
		err = bind(
			net->fd,
			(struct sockaddr *) &(net->addr),
			net->addr_len
		);

		if ( err < 0 )
			return NET_ERR_INIT_BIND;
		vec_init( &(net->data) );
	}

	return NET_OK;
}


// Lazy init for server and client
int
net_client ( struct net * net, const short port, const char * ip6, int version )
{
	return net_init(net, port, ip6, NET_CLIENT, version );
}

int
net_server ( struct net * net, const short port, const char * ip6, int version )
{
	return net_init(net, port, ip6, NET_SERVER, version);
}


// sendto with net structure
ssize_t
net_write ( struct net * net, const void * buf, size_t len, int flags )
{
	//some check
	if ( !net ) return NET_FAIL;
	if ( net->mode != NET_SERVER && net->mode != NET_CLIENT ) return NET_FAIL;
	if ( net->version != NET_IPV6 && net->version != NET_IPV4 ) return NET_FAIL;

	int ret = NET_OK;

	if ( net->mode == NET_CLIENT )
	{
		char address[64];
		inet_ntop(
			net->version == 4 ? AF_INET : AF_INET6,
			(void*) &net->addr.v4.sin_addr,
			address,
			sizeof(address)
		);
		printf("send to address %s and %d port.\n", address, ntohs(net->addr.v4.sin_port) );

		ret = sendto(net->fd, buf, len, flags, (struct sockaddr *)&net->addr, net->addr_len);
	}
	else
	{
		if ( !!net->current && ( net->current_len == sizeof(struct sockaddr_in6) || net->current_len == sizeof(struct sockaddr_in)) )
		{
			ret = sendto(net->fd, buf, len, flags, (void*) net->current, net->current_len);
		}
		else
		{
			printf("net->current is not net->current! Liar!\n");
			return NET_FAIL;
		}
	}

	return ret;
}

void net_set_timeout ( struct net * net, struct timeval * t ) { net->timeout = t; }

// recvfrom with net structure
int
net_read ( struct net * net, void * buf, size_t len, int flags )
{
	//some check
	if ( !net ) return NET_FAIL;
	if ( net->mode != NET_SERVER && net->mode != NET_CLIENT ) return NET_FAIL;
	if ( net->version != NET_IPV6 && net->version != NET_IPV4 ) return NET_FAIL;

	int ret = -1;
	char addr_buf[32];
	
	for ( int i = 0 ; i < 32 ; i++ )
		addr_buf[i] = 0x00;
	
	net->current_len = 32;

	int fd = net->fd;
	fd_set fd_read;
	FD_ZERO( &fd_read );
	FD_SET( fd, &fd_read );

	int ret_select = select(fd + 1 , &fd_read, NULL, NULL, net->timeout );
	if ( ret_select == -1 )
	{
		orz(" - select() failed - ");
		return NET_FAIL;
	}
	else if ( ret_select == 0 )
	{
		return 0;
	}

	if ( FD_ISSET( net->fd, &fd_read ) )
	{
		ret = recvfrom( net->fd, buf, len, flags, (struct sockaddr *)addr_buf, &net->current_len );
	}
	if ( net->current_len == sizeof(struct sockaddr_in6) ||  net->current_len == sizeof(struct sockaddr_in))
	{
		if (net->current)
			free(net->current);
	
		net->current = malloc( net->current_len );
		memcpy( net->current, addr_buf, net->current_len );
	}
	else
	{
		return 0;  
	}


	return ret;
}


/*
 *		net_read2()
 *		timeout is the value of net1->timeout ( i.e. net2->timeout is ignored)
 */
int
net_read2 ( struct net * net1, struct net * net2, void * buf, size_t len, int flags )
{
	if ( !net1 || !net2 ) return NET_FAIL;

	if ( net1->mode != NET_SERVER && net1->mode != NET_CLIENT ) return NET_FAIL;
	if ( net2->mode != NET_SERVER && net2->mode != NET_CLIENT ) return NET_FAIL;
	if ( net1->version != NET_IPV6 && net1->version != NET_IPV4 ) return NET_FAIL;
	if ( net2->version != NET_IPV6 && net2->version != NET_IPV4 ) return NET_FAIL;

	int ret = 1;
	int max = 0;

	unsigned char addr_buf[sizeof(struct sockaddr_in6)];


	fd_set fd_read;
	FD_ZERO( &fd_read );
	FD_SET(  net1->fd, &fd_read );
	FD_SET(  net2->fd, &fd_read );

	net1->current_len = 32;
	net2->current_len = 32;

	//max fd for select
	max = ( net1->fd > net2->fd ) ? net1->fd : net2->fd;

	int ret_select = select( max + 1, &fd_read, NULL, NULL, net1->timeout );
	if ( ret_select == 0 )
	{
		return 0;
	}
	else if ( ret_select == -1 )
	{
		orz("Error - select");
		return -1;
	}

	for (int i = 0; i < 2; i++) {
		struct net* net;

		if (i == 0)
			net = net1;
		else
			net = net2;

		if ( FD_ISSET( net->fd, &fd_read ) )
		{
			ret = recvfrom( net->fd, buf, len, flags, (struct sockaddr *)addr_buf, &net->current_len );
			
			if ( net->current_len == sizeof(struct sockaddr_in6) ||  net->current_len == sizeof(struct sockaddr_in))
			{
				if ( !net->current )
					net->current = malloc( sizeof(union s_addr) );

				memcpy( net->current, addr_buf, net->current_len );

				printf(" port %d - %d - %d\n", ntohs( net->current->v4.sin_port), ntohs( net->addr.v4.sin_port), i+1 );printf(">>\n");
				return ret;
			}
			else
			{
				orz("read2 - bad size");
				return 0;
			}
		}
	}

	return ret;
}


int
net_read_vec ( struct net * net1, struct net * net2, vec_void_t * nets, void * buf, size_t len, int flags )
{
	//if ( nets->length == 0 ) return net_read2(net1, net2, buf, len, flags);

	int ret = 1;
	int max = 0;

	unsigned char addr_buf[sizeof(struct sockaddr_in6)];
	struct net* net;
	int i;

	fd_set fd_read;
	FD_ZERO( &fd_read );


	max = ( max > net1->fd ) ? max : net1->fd;
	max = ( max > net2->fd ) ? max : net2->fd;
	FD_SET( net1->fd, &fd_read );
	FD_SET( net2->fd, &fd_read );

	vec_foreach ( nets, net, i )
	{
		printf(" i :: %d\n", i);
		max = ( max > net->fd ) ? max : net->fd;
		FD_SET( net->fd, &fd_read );
	}
	
	//max fd for select

	int ret_select = select( max + 1, &fd_read, NULL, NULL, net1->timeout );
	if ( ret_select == 0 )
	{
		return 0;
	}
	else if ( ret_select == -1 )
	{
		orz("Error - select");
		return -1;
	}

	printf("fghjk\n");

	for ( i = 0 ; i < ( nets->length + 2) ; i++ )
	{
		if ( i == 0 )
			net = net1;
		else if ( i == 1 )
			net = net2;
		else 
			net = nets->data[i-2];


		if ( FD_ISSET( net->fd, &fd_read ) )
		{
			ret = recvfrom( net->fd, buf, len, flags, (struct sockaddr *)addr_buf, &net->current_len );
			
			if ( net->current_len == sizeof(struct sockaddr_in6) ||  net->current_len == sizeof(struct sockaddr_in))
			{
				if ( !net->current )
					net->current = malloc( sizeof(s_addr) );

				memcpy( net->current, addr_buf, net->current_len );

				printf(" port %d - %d - %d\n", ntohs( net->current->v4.sin_port), ntohs( net->addr.v4.sin_port), i+1 );printf(">>\n");
				return ret;
			}
			else
			{
				orz("read_vec - bad size");
				return 0;
			}
		}
	}

	return ret;
}

int
net_shutdown ( struct net * net )
{
	int err = NET_OK;
	if ( net->mode == NET_SERVER )
	{
		printf("client vector\n");

		int i = 0;
		void * e;
		vec_foreach ( &(net->data), e, i )
		{
			if ( e != NULL )
				free(e);
		}
		vec_deinit( &(net->data) );
	}

	if (net->current)
		free(net->current);

	err = shutdown(net->fd, SHUT_RDWR);

	return err;
}
