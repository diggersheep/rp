#include <stdio.h>
#include <string.h>

#include "net.h"
#include "common.h"
#include "hash.h"


// TEMPORAIRE !!!!!!!!!!!!!!
// TEMPORAIRE !!!!!!!!!!!!!!
// TEMPORAIRE !!!!!!!!!!!!!!
int send_put (struct net * net, const char * hash, int type );
// TEMPORAIRE !!!!!!!!!!!!!!
// TEMPORAIRE !!!!!!!!!!!!!!
// TEMPORAIRE !!!!!!!!!!!!!!


int send_put_all ( struct net * net, HashData * hd )
{
	int err = 0;
	err = send_put(net, (char *)hd->digest, 0);

/*
	for ( int i = 1 ; i < hd->chunkDigests.length ; i ++ )
	{
		err = err | send_put(net, hd->chunkDigests.data[i], 0 );
	}
*/

	return err;
}

int send_put ( struct net * net, const char * hash, int type )
{
	if ( type != 0 && type != 1 )
		return 1;

	//fichier = 50
	//fichier = 51
	//cli     = 55
	//frag    = 60

	RequestPut r;
	// c / size / hash(32) / index

	if ( hash )
	{
		r.type = REQUEST_PUT;
		printf(" >> %d\n", REQUEST_PUT);

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
			
			return 1;
		}
		else
		{
			printf("ERR put hash :  ");
			print_hash((unsigned char *)hash);
			printf("\n");
			
		}
	}

	return 0;
}

int
main ( int argc, const char * argv[] )
{

	if ( argc < 2 )
		return 1;


//	char filename[] = "hash-file.c";
	char filename[] = "/home/alex/Téléchargements/IMG_20161203_100138909.jpg";
	short port = 9000;
	char ip[] = "192.168.0.12";

	struct net net;
	int err = net_init(&net, port, ip, NET_CLIENT, NET_IPV4);
	net_error(err);

	HashData * hd = NULL;
	hd = hash_data_new(filename);


	if ( strcmp( argv[1], "print" ) == 0 )
	{
	//	char c = REQUEST_PRINT; 
	//	net_write( &net, &c, 1 ,0 );
	}
	else if ( strcmp( argv[1], "put" )  == 0 )
	{
		printf("put all : %d\n",  send_put_all( &net, hd ));
		hash_data_free(hd);
	}
	else if ( strcmp( argv[1], "list" )  == 0 )
	{
		/*
		RequestPut r;
		r.type = REQUEST_LIST;

			
		for ( int i = 0 ; i < hd->chunkDigests.length ; i ++ )
		{
			if ( hd->chunkDigests.data[i] )
			{
				memcpy( &(r.chunk_hash), hd->chunkDigests.data[i], 32 );
				if ( net_write( &net, &r, sizeof(r), 0) != -1)
				{
					char c[33];
					c[32] = 0x00;
					memcpy( &(r.chunk_hash), c, 32 );
	
				}
			}
		}
		hash_data_free(hd);*/
	}
	else if ( strcmp( argv[1], "db" )  == 0 )
	{
		char c [] = "bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p";
		net_write( &net, c, sizeof("bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p"), 0);
	}

	return 0;
}
