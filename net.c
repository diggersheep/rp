#include "net.h"

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
net_init(
	struct net * net, // struct net uninitialize
	const short port, // port 
	const char * ip,  // IP v4/v6 in string
	int mode,         // NET_CLIENT | NET_SERVER
	int version)
{
	int err = 0; // error return

	// check mode
	if ( mode != NET_SERVER && mode != NET_CLIENT)
		return NET_ERR_INIT_MODE;

	//init struct
	net->addr_len = sizeof(net->addr);
	net->mode = mode;
	net->fd = -1;

	//set in memory sockaddr_in6 + init sockaddr_in6
	if (version == NET_IPV6 )
	{		
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v6.sin6_family = AF_INET6;
		net->addr.v6.sin6_port   = htons(port);
	}
	else // set in memory sockaddr_in + init socckaddr_in
	{
		memset( (char*)&(net->addr), 0, sizeof(net->addr));
		net->addr.v4.sin_family = AF_INET;
		net->addr.v4.sin_port   = htons(port);
	}

	// copy of @ip
	if ( version == NET_IPV6 )
	{
		err = inet_pton( AF_INET6, ip, &(net->addr.v6.sin6_addr) );
	}
	else
	{
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

	ssize_t ret = 0;// return value

	ret = sendto(
			net->fd,
			buf,
			len,
			flags,
			(struct sockaddr *) &(net->addr),
			net->addr_len
		);
	return ret;
}

// recvfrom with net structure
ssize_t
net_read ( struct net * net, void * buf, size_t len, int flags )
{
	//some check
	if ( !net ) return NET_FAIL;
	if ( net->mode != NET_SERVER && net->mode != NET_CLIENT ) return NET_FAIL;

	ssize_t ret = 0;//return value
	ret = recvfrom(
			net->fd,
			buf,
			len,
			flags,
			(struct sockaddr *) &(net->addr),
			&(net->addr_len)
		);

	return ret;
}

