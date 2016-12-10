#include "client.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int recv_put_ack ()//( struct net * net, RequestPutAck * datagram )
{
	


	return 0;
}


int send_put_all ( struct net * net, struct net * srv, const HashData * hd )
{
	int err = 0;
	err = send_put(net, srv, (char *)hd->digest, 0);

/*
	for ( int i = 1 ; i < hd->chunkDigests.length ; i ++ )
	{
		err = err | send_put(net, hd->chunkDigests.data[i], 0 );
	}
*/	

	return err;
}

int send_put ( struct net * net, struct net * srv, const char * hash, int type )
{
	if ( type != 0 && type != 1 )
		return 1;

	//fichier = 50
	//fichier = 51
	//cli     = 55
	//frag    = 60

	RequestPut r;

	char buf[512];
	buf[0] = 0x00;
	int ok = 0;

	fd_set rr;

	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 100;

	for (int i = 0;; i++)
	{
		if ( i == 1000*1000*10 )
			break;

		FD_ZERO( &rr );
		
		FD_SET(  srv->fd, &rr );
		FD_SET(  net->fd, &rr );

		if ( select( srv->fd + 1 , &rr, NULL, NULL, &t ) < 0)
		{
			printf("FAIL SELECT\n");
			break;
		}
		else
		{

			if ( i % 1000000 == 0 )
			{
				putchar('.');
				fflush(stdout);
			}			
		}



		if ( FD_ISSET( srv->fd, &rr ) )
		{


			printf("READ\n");

			if ( net_read(srv, buf, 512, 0)  != -1)
			{
				printf("RRRRREEEEEEEAAAAAAAADDDDDDDD\n");
				break;
			}
			else
			{
				printf("Fuck\n");
			}


			
		}


		if ( FD_ISSET( net->fd, &rr ) )
		{

			
			printf("READ\n");

			char * buf[512];
			if ( net_read(srv, buf, 512, 0)  != -1)
			{
				printf("RRRRREEEEEEEAAAAAAAADDDDDDDD\n");
				break;
			}
			else
			{
				printf("Fuck\n");
			}


			
		}

		if ( ok == 0 )
		{

			if ( hash )
			{
				r.type = REQUEST_PUT;

				if ( type == 0 )
					r.hash_segment.c = 50; //fichier
				else
					r.hash_segment.c = 51; //hash

				r.hash_segment.size = 32; //obvious
				memcpy( &(r.hash_segment.hash), hash, r.hash_segment.size );
					
				if ( net->version == NET_IPV4 )
				{
					r.client_segment.v4.c = 55;
					r.client_segment.v4.ipv  = 6;
					r.client_segment.v6.port = net->addr.v4.sin_port;
					memcpy( &(r.client_segment.v4.address), &(net->addr.v4.sin_addr), sizeof(r.client_segment.v4.address) );
				}
				else
				{
					r.client_segment.v6.c    = 55;
					r.client_segment.v6.ipv  = 18;
					r.client_segment.v6.port = net->addr.v6.sin6_port;
					memcpy( &(r.client_segment.v6.address), &(net->addr.v6.sin6_addr), sizeof(r.client_segment.v6.address) );
				}


				if ( net_write( net, &r, sizeof(r), 0) != -1)
				{
					char c[33];
					c[32] = 0x00;
					printf("OK  put hash :  ");
					print_hash((unsigned char *)hash);
					printf("\n");
				}
				else
				{

					printf("ERR put hash :  ");
					print_hash( (unsigned char *)hash );
					printf("\n");
					return 1;
				}
			}

			ok = 1;
		}
	}
	printf("\n");




	return 0;
}






int
main ( int argc, const char * argv[] )
{

	if ( argc < 2 )
		return 1;


	char filename[] = "client.c";
//	char filename[] = "/home/alex/Téléchargements/IMG_20161203_100138909.jpg";
	short port = 9000;
	char ip[] = "192.168.0.12";


	char buf[CHUNK_SIZE+64];
	buf[0] = 0x00; //safety



	struct net net;
	struct net srv;
	int err = net_init(&net, port, ip, NET_CLIENT, NET_IPV4);
	net_error(err);
	err = net_init(&srv, port, "0.0.0.0", NET_SERVER, NET_IPV4);
	net_error(err);

	if (net.version == 4)
		srv.current = (struct sockaddr *)&(net.addr.v4);
	else
		srv.current = (struct sockaddr *)&(net.addr.v6);


	HashData * hd = NULL;
	hd = hash_data_new(filename);

	if ( strcmp( argv[1], "print" ) == 0 )
	{
		char c = REQUEST_PRINT; 
		net_write( &net, &c, 1 ,0 );
	}
	else if ( strcmp( argv[1], "put" )  == 0 )
	{

		send_put_all(&net, &srv, hd);

		hash_data_free(hd);
	}
	else if ( strcmp( argv[1], "list" )  == 0 )
	{
	}
	else if ( strcmp( argv[1], "db" )  == 0 )
	{
		char c [] = "bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p";
		net_write( &net, c, sizeof("bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p"), 0);
	}

	return 0;
}
