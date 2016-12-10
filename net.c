#include "net.h"
#include "vec/vec.h"


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
			fprintf( stderr, "  [ERR] - net_init() - cannot convert IPv6 address.\n");
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
net_init   ( struct net * restrict net, const short port, const char * restrict ip, int mode, int version )
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

void net_set_timeout ( struct net * net, struct timeval * t )
{
	net->timeout = t;
}

// recvfrom with net structure
ssize_t
net_read ( struct net * net, void * buf, size_t len, int flags )
{
	//some check
	if ( !net ) return NET_FAIL;
	if ( net->mode != NET_SERVER && net->mode != NET_CLIENT ) return NET_FAIL;
	if ( net->version != NET_IPV6 && net->version != NET_IPV4 ) return NET_FAIL;

	ssize_t ret = 1;
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
		printf("  Error - select.\n");
		return NET_FAIL;
	}
	else if ( ret_select == 0 )
	{
		printf("  Warning - select timeout.\n");
		return 0;
	}

	ret = recvfrom( net->fd, buf, len, flags, (struct sockaddr *)addr_buf, &net->current_len );

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

	err = shutdown(net->fd, SHUT_RDWR);

	return err;
}
