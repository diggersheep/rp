#include <stdio.h>
#include <string.h>

#include "net.h"
#include "common.h"
#include "hash.h"
#include "hash-file.h"


int send_put_all ( struct net * net, const char * filename )
{

}

int send_put ( struct net * net, const char * hash )
{
	
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
		char c = REQUEST_PRINT; 
		net_write( &net, &c, 1 ,0 );
	}
	else if ( strcmp( argv[1], "put" )  == 0 )
	{
		RequestPut r;
		r.type = REQUEST_PUT;


		for ( int i = 0 ; i < hd->chunkDigests.length ; i ++ )
		{
			if ( hd->chunkDigests.data[i] )
			{
				memcpy( &(r.chunk_hash), hd->chunkDigests.data[i], 32 );
				if ( net_write( &net, &r, sizeof(r), 0) != -1)
				{

					printf("PUT ");
					print_hash(hd->chunkDigests.data[i]);
					printf("\n");
				}
			}
		}
		hash_data_free(hd);

	}
	else if ( strcmp( argv[1], "list" )  == 0 )
	{
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
		hash_data_free(hd);
	}
	else if ( strcmp( argv[1], "db" )  == 0 )
	{
		char c [] = "bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p";
		net_write( &net, c, sizeof("bonjour tout le monde ! ça va ? oui, bien et toi ? Tranquille !! :p"), 0);
	}

	return 0;
}
