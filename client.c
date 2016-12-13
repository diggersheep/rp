#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "sha256/sha256.h"

#include "net.h"
#include "common.h"
#include "hash.h"
#include "debug.h"

#include "client.h"

#include <time.h>

#define CMD_PUT 1
#define CMD_GET 2
#define CMD_PRINT 3
#define CMD_LIST 4
#define CMD_DEBUG 99

static char address_schar_buffer[128];
static const char*
address_schar(int family, void* address)
{
	inet_ntop(family, address, address_schar_buffer, sizeof(address_schar_buffer));
	return address_schar_buffer;
}

/**
 * Yahaloo,
 *
 * To whomever is reading this file, here’s the basic organisation of most of
 * the client’s code:
 *
 *   - all functions that are used to send a specific request with specific data
 *     are called send_<request_name>.
 *     To understand them, you want to look at their matching structure in
 *     `common.h`.
 *   - all functions that are used to parse a specific request and take action
 *     based on its content are called handle_<request_name>.
 *     Again, you’ll want to look at `common.h`. You may also need to take a
 *     look at the project’s subject to understand the behavioral’s details.
 *
 * event_loop() is where most of the action takes place. It’s the main loop,
 * and basically all incoming requests are handled in it.
 */

int
send_ec_str(struct net* net, const char* str)
{
	char buffer[1024];
	RequestEC* r = (void*) buffer;

	r->type = REQUEST_EC;
	r->subtype = 0;

	int len = snprintf((void*) r->data, sizeof(buffer) - sizeof(*r), "%s", str);

	r->size = len;

	net_write(net, r, sizeof(*r) + r->size, 0);

	return 0;
}

void
send_get_client(RegisteredFile* rf, vec_void_t* connected_clients)
{
	RequestGetClient r;
	int i, j, k;
	SegmentClient* client;
	struct net* net;

	r.type = REQUEST_GET_CLIENT;
	r.size = sizeof(SegmentFileHash) + sizeof(SegmentChunkHash);

	r.file_hash_segment.c = 50;
	memcpy(r.file_hash_segment.hash, rf->hash_data->digest, 32);

	r.chunk_hash_segment.size = 34;
	r.chunk_hash_segment.c = 51;

	vec_foreach (connected_clients, net, i) {
		vec_foreach (&rf->related_clients, client, j) {
			int sin_family = client->v4.ipv == 6 ? AF_INET : AF_INET6;
			int equal = net->addr.v4.sin_family == sin_family;
			equal = equal && net->addr.v4.sin_port == client->v4.port;
			equal = equal && (0 == memcmp(&net->addr.v4.sin_addr, &client->v4.address, client->v4.ipv == 6 ? 4 : 16));

			if (equal) {
				msg_out("GET-CLIENT", "%s:%d",
					address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
					net->current->v6.sin6_port);

				unsigned char* hash;
				vec_foreach (&rf->hash_data->chunkDigests, hash, k) {
					int l = 0;

					for (; l < 1000; l++) {
						if (!((int*) rf->received_fragments.data[k])[l]) {
							r.chunk_hash_segment.index = k;

							memcpy(r.chunk_hash_segment.hash, hash, 32);

							net_write(net, &r, sizeof(r), 0);

							break;
						}
					}
				}

				break;
			}
		}
	}
}

int
send_list(
	const unsigned char * hash,
	RegisteredFile * rf,
	vec_void_t* connected_clients
)
{
	if (!hash)
		return NET_FAIL;

	int ret = 0;

	SegmentClient * client;

	if ( rf->related_clients.length < 1 ) {
		orz("No clients for \"%s\"", hash_data_schar(hash));
		return -1;
	}

	client = rf->related_clients.data[rand() % rf->related_clients.length];

	int sin_family = client->v4.ipv == 6 ? AF_INET : AF_INET6;

	char address[64];
	inet_ntop(sin_family, client->v6.address, address, sizeof(address));

	msg_out("LIST", "%s:%d",
		address_schar(client->v6.ipv == 6 ? AF_INET : AF_INET6, &client->v6.address),
		ntohs(client->v6.port));

	msg_file_hash_out( hash_data_schar(hash) );
	// fprintf(stdout, "\033[01;34m %-16s >> \033[01;37m", packet);

	struct net* peer = NULL;
	int already_registered = 0;
	int i;
	vec_foreach (connected_clients, peer, i) {
		int equal = peer->addr.v4.sin_family == sin_family;
		equal = equal && peer->addr.v4.sin_port == client->v4.port;
		equal = equal && (0 == memcmp(&peer->addr.v4.sin_addr, &client->v4.address, client->v4.ipv == 6 ? 4 : 16));

		if (equal) {
			already_registered = 1;

			break;
		}
	}

	if (!already_registered) {
		peer = malloc(sizeof(*peer));

		int error = net_init_raw(
			peer,
			client->v4.port,
			(void*) &client->v4.address,
			NET_CLIENT,
			(client->v4.ipv == 6) ? NET_IPV4 : NET_IPV6
		);

		net_error(error);
	}

	struct timeval t;
	t.tv_sec  = 1;
	t.tv_usec = 0;

	unsigned char buffer[sizeof(RequestList)];
	RequestList * rq = (void*) buffer;

	rq->type = REQUEST_LIST;

	rq->hash.c    = 50;
	rq->hash.size = 32;
	memcpy(
		rq->hash.hash,
		hash,
		32
	);

	ret = net_write( peer, rq, sizeof(*rq), 0);
	send_ec_str( peer, "bonjour");

	if (!already_registered)
		vec_push(connected_clients, peer);

	return ret;
}

void
handle_list ( struct net* net, char * buffer, vec_void_t * registered_files , struct net * server )
{
	RequestList    * rq = (void*) buffer;
	RequestListAck * rp = (void*) buffer;

	int i;
	RegisteredFile * rf;
	int check = 0;

	msg_in("LIST", "%s:%d",
		address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
		ntohs(net->current->v6.sin6_port));

	if ( rq->hash.c != 50 )
	{
		wtf("LIST> Wrong file hash %s", hash_data_schar(rq->hash.hash));
		send_ec_str(server, "Wrong file Hash !");

		return;
	}

	vec_foreach ( registered_files, rf, i)
	{
		check = memcmp( rf->hash_data->digest, rq->hash.hash, 32 );	

		if (check) {
			check = rf->received_fragments.length > 0;

			if (check) {
				break;
			} else {
				send_ec_str(server, "Don’t have it! Nico Nico-nii again some other time.");

				return;
			}
		}
	}
	if ( check != 0 )
	{
		wtf("LIST> we don't have %s", hash_data_schar(rq->hash.hash));
		send_ec_str(server, "No list !");
		return;
	}

	char address[64];
	address[0] = 0x00;
	inet_ntop(
		server->version == NET_IPV4 ? AF_INET : AF_INET6,
		&server->current->v4.sin_addr,
		address,
		sizeof(address)
	);

	rf = registered_files->data[i-1];

	rp->type = REQUEST_LIST_ACK;
	rp->size = rf->hash_data->chunkDigests.length;
	unsigned char * hash;

	vec_foreach( &rf->hash_data->chunkDigests, hash, i )
	{
		rp->data[i].c    = 51;
		rp->data[i].size = 32;
		memcpy(
			rp->data[i].hash,
			hash,
			32
		);
		rp->data[i].index = i;
	}

	msg_out("LIST/ACK", "%s:%d",
		address_schar(server->current->v6.sin6_family, &server->current->v6.sin6_addr),
		ntohs(server->current->v6.sin6_port));

	net_write( server, rp, sizeof(*rp) + ((i+1) * sizeof(SegmentChunkHash)) , 0 );
}

void handle_list_ack ( struct net* net, char * buffer, vec_void_t * registered_files )
{
	RequestListAck * rq = (void*) buffer;

	msg_in("LIST/ACK", "%s:%d",
		address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
		ntohs(net->current->v6.sin6_port));

	if ( rq->file_hash_segment.c != 50 )
	{
		orz("Bad file hash segment ! - ");
		return;
	}

	int i;
	RegisteredFile * rf;
	vec_foreach ( registered_files, rf, i )
	{
		if ( memcmp( rq->file_hash_segment.hash, rf->hash_data->digest, 32) != 0 )
		{
			orz("bad file hash %s - ", hash_data_schar(rq->file_hash_segment.hash));
			return;
		}
		else
		{

			msg_file_hash_in( hash_data_schar(rq->file_hash_segment.hash) );
			break;
		}
	}

	short size = rq->size;
	if ( size <= 0 )
	{
		orz("Bad size for file hash (%d)", size);
		return;
	}

	for ( int i = 0 ; i < size ; i++ )
	{
		if ( rq->data[i].c != 51 )
		{
			orz("Broken segment.");
			return;
		}
		if ( rq->data[i].size != 32 )
		{
			orz("Broken segment.");
			return;
		}
		//check for segment
		int * check_chunk = malloc(sizeof(int) * 1000);
		memset(check_chunk, 0, 1000);
		vec_push( &rf->received_fragments, check_chunk );

		//chunk hash
		char * chunk_hash = malloc(32);
		memcpy( chunk_hash, &rq->data[i].hash, 32);
		vec_push( &rf->hash_data->chunkDigests, chunk_hash );

		msg_chunk_hash_in( hash_data_schar( (unsigned char*) chunk_hash ), rq->data[i].index );
//		for ( int j = 0 ; j < 1000 ; j++ )
//			send_get_client(buffer, );
	}

	//mode get client
	rf->timeout = 5;
	rf->status  = STATUS_GET_CLIENT;
	
}

int
send_get_cli ( struct net* net, const HashData* hd, short index )
{
	if ( index >= hd->chunkDigests.length || index < 0 )
		return -1;

	int err = 0;

	char buffer[sizeof(RequestGetClient)];
	RequestGetClient * rq = (void*) buffer;

	rq->type   = 100;

	//hash file
	rq->file_hash_segment.c    = 50;
	rq->file_hash_segment.size = 32;
	memcpy(
		rq->file_hash_segment.hash,
		hd->digest,
		32
	);

	//chunk hash
	rq->chunk_hash_segment.c     = 51;
	rq->chunk_hash_segment.index = index; 
	memcpy(
		rq->chunk_hash_segment.hash,
		hd->chunkDigests.data[index],
		32
	);

	net_write(net, (void*)rq, sizeof(*rq), 0);

	return err;
}

int
send_put(struct net* net, struct net* srv, const HashData* hd)
{
	int err = 0;
	char buffer[sizeof(RequestPut)];
	RequestPut* rq = (void*) buffer;

	rq->type = REQUEST_PUT;

	rq->hash_segment.c = 50;
	rq->hash_segment.size = 32;
	memcpy((void*) rq->hash_segment.hash, hd->digest, sizeof(rq->hash_segment.hash));

	rq->client_segment.v4.c = 55;
	if (net->version == NET_IPV4)
		rq->client_segment.v4.ipv = 6;
	else
		rq->client_segment.v4.ipv = 18;

	struct sockaddr_in* saddr = (struct sockaddr_in*) &net->addr;
	rq->client_segment.v4.port = srv->addr.v4.sin_port;

	memcpy(
		(void*) rq->client_segment.v6.address,
		&saddr->sin_addr,
		net->version == 4 ? 4 : 16
	);

	msg_out("PUT", "%s:%d", address_schar(saddr->sin_family, &rq->client_segment.v6.address), ntohs(saddr->sin_port));

	net_write(net, (void*) rq, sizeof(*rq), 0);

	return err;
}


int
send_keep_alive(struct net* net, const HashData * hd)
{
	int err = 0;
	char buffer[sizeof(RequestKeepAlive)];
	RequestPut* rq = (void*) buffer;

	rq->type = REQUEST_KEEP_ALIVE;

	rq->hash_segment.c = 50;
	rq->hash_segment.size = 32;
	memcpy((void*) rq->hash_segment.hash, hd->digest, sizeof(rq->hash_segment.hash));


	unsigned char address[64];
	memcpy(
		address,
		&net->addr.v4.sin_addr,
		net->version == 4 ? 4 : 16
	);

	msg_out("KEEP-ALIVE", "%s:%d",
		address_schar(net->addr.v4.sin_family, address),
		ntohs(net->addr.v6.sin6_port));

	net_write(net, (void*) rq, sizeof(*rq), 0);

	return err;
}

void
send_get(struct net* tracker, struct net* server, const HashData* hd)
{
	RequestGet r;
	struct sockaddr_in6* saddr = &server->addr.v6;

	r.type = REQUEST_GET;

	r.hash_segment.c = 50;
	r.hash_segment.size = 32;
	memcpy(&r.hash_segment.hash, hd->digest, 32);

	r.client_segment.v4.c = 55;
	r.client_segment.v4.ipv = server->version == 4 ? 6 : 18;
	r.client_segment.v6.port = saddr->sin6_port;
	memcpy(&r.client_segment.v6.address, (void*) &saddr->sin6_addr,
		server->version == 4 ? 4 : 16);

	msg_out("GET", "%s:%d",
		address_schar(tracker->addr.v6.sin6_family, &tracker->addr.v6.sin6_addr),
		ntohs(tracker->addr.v6.sin6_port));

	net_write(tracker, (void*) &r, sizeof(r), 0);
}

const char*
status_string(RegisteredFile* rf)
{
	switch (rf->status) {
		case STATUS_PUT:
			return "PUT";
		case STATUS_KEEP_ALIVE:
			return "KEEP-ALIVE";
		case STATUS_GET:
			return "GET";
		case STATUS_LIST:
			return "LIST";
		case STATUS_GET_CLIENT:
			return "GET_CLIENT";
	}
}



void
handle_timeout(struct net* tracker, struct net* server, vec_void_t* registered_files, vec_void_t* connected_clients)
{
	int i;
	RegisteredFile* rf;

	vec_foreach(registered_files, rf, i) {
		if (--rf->timeout == 0) {
			switch (rf->status) {
				case STATUS_PUT:
					send_put(tracker, server, rf->hash_data);
					rf->timeout = 5;
					break;
				case STATUS_KEEP_ALIVE:
					send_put(tracker, server, rf->hash_data);
					rf->status = STATUS_PUT;
					rf->timeout = 5;
					break;
				case STATUS_GET:
					send_get(tracker, server, rf->hash_data);
					rf->timeout = 5;
					break;
				case STATUS_LIST:
					send_list( rf->hash_data->digest, rf, connected_clients );
					break;
				case STATUS_GET_CLIENT:
					send_get_client(rf, connected_clients);
					break;
			}
		} else if (rf->status == STATUS_KEEP_ALIVE || rf->status == STATUS_GET) {
			if (rf->timeout <= 30 && rf->timeout % 5 == 0) {
				send_keep_alive(tracker, rf->hash_data);
			}
		}
	}
}

void
handle_ec(char* buffer, int size)
{
	RequestEC* r = (void*) buffer;

	if ((unsigned) size < sizeof(*r) || (unsigned) size < sizeof(*r) + r->size) {
		orz("received broken EC, datagram was too short");
		return;
	}

	if (r->subtype == 0) {
		printf("\033[01;34m EC/MSG           << \033[01;37m");
		fflush(stdout);
		write(0, (const char*) r->data, r->size);
		printf("\n");
	} else {
		wtf("Received unhandled EC type.");
	}
}

void
handle_put_ack(char* buffer, int size, vec_void_t* registered_files)
{
	RequestPutAck* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) size < sizeof(*r)) {
		orz("Received broken PUT/ACK, datagram too short");
		return;
	}

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->hash_segment.hash, 32)) {
			rf->timeout = 60;
			rf->status = STATUS_KEEP_ALIVE;

			return;
		}
	}
}

void
handle_put_error(struct net* net, char* buffer, int size, vec_void_t* registered_files)
{
	RequestPutError* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) size < sizeof(*r)) {
		orz("Received broken PUT/ERROR, datagram too short");

		return;
	}

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->hash_segment.hash, 32)) {
			msg_in("PUT/ERR", "%s:%d",
				address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
				ntohs(net->current->v6.sin6_port));

			rf->timeout = 60;
			rf->status = STATUS_KEEP_ALIVE;

			return;
		}
	}
}

void
handle_keep_alive_ack(char* buffer, int size, vec_void_t* registered_files)
{
	RequestKeepAliveAck* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) size < sizeof(*r)) {
		orz("Received broken KEEP-ALIVE/ACK, datagram too short");

		return;
	}

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->hash_segment.hash, 32)) {
			rf->timeout = 60;

			return;
		}
	}
}

void
handle_keep_alive_error(struct net* tracker, struct net* server, char* buffer, int size, vec_void_t* registered_files)
{
	RequestKeepAliveError* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) size < sizeof(*r)) {
		orz("Received broken KEEP-ALIVE/ERROR, datagram too short");
		return;
	}

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->hash_segment.hash, 32)) {
			send_put(tracker, server, rf->hash_data);

			rf->timeout = 60;
			rf->status = STATUS_PUT;

			return;
		}
	}
}

void
handle_get_ack( struct net* net, char* buffer, int count, vec_void_t* registered_files, vec_void_t* connected_clients)
{
	RequestGetAck* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) count < sizeof(*r)) {
		orz("Received broken GET/ACK, datagram too short");
		return;
	}

	msg_out("GET/ACK", "%s:%d",
		address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
		ntohs(net->current->v6.sin6_port));

	msg_file_hash_out( hash_data_schar(r->hash_segment.hash) );

	count -= sizeof(*r);

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->hash_segment.hash, 32)) {
			char* current_offset;
			int j;
			SegmentClient* s;

			vec_foreach_rev (&rf->related_clients, s, j) {
				free(s);

				vec_pop(&rf->related_clients);
			}

			current_offset = (char*) r->clients;

			for (; r->count > 0; r->count--) {

				s = (SegmentClient*) current_offset;

				size_t segment_size =
					s->v4.ipv == 6 ? sizeof(SegmentClient4) : sizeof(SegmentClient6);

				count -= segment_size;

				if (count < 0) {
					orz("broken GET/ACK received and partly handled");
					break;
				}

				void* clone = malloc(segment_size);

				memcpy(clone, current_offset, segment_size);

				current_offset += segment_size;

				vec_push(&rf->related_clients, clone);

				char address[64];
				inet_ntop(
					s->v4.ipv == 6 ? AF_INET : AF_INET6,
					(void*) &s->v4.address, address, sizeof(address)
				);

			}

			rf->timeout = 30;

			send_list( rf->hash_data->digest, rf, connected_clients );
			return;
		}
	}
}

void
handle_get_client(struct net* net, char* buffer, int count, vec_void_t* registered_files)
{
	RegisteredFile* rf;
	RequestGetClient* r = (void*) buffer;
	int i;

	if ((unsigned) count < sizeof(*r)) {
		orz(" - received broken GET-CLIENT - ");
		return;
	}

	msg_in("GET-CLIENT", "%s:%d",
		address_schar(net->addr.v6.sin6_family, &net->addr.v6.sin6_addr),
		ntohs(net->addr.v6.sin6_port));

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->chunk_hash_segment.hash, 32)) {
			RequestGetClientAck* answer = (void*) buffer;

			FILE* f = fopen(rf->filename, "r");

			if (!f) {
				orz("fopen");
				return;
			}

			printf("Preparing to send fragments.\n");

			fseek(f, r->chunk_hash_segment.index * CHUNK_SIZE, SEEK_SET);

			answer->type = REQUEST_GET_CLIENT_ACK;

			answer->fragment.index = 0;

			/* FIXME: does not wurk, update once handle_list_ack is done */
			for (int j = 0; j < CHUNK_SIZE / FRAGMENT_SIZE; j++) {
				int r = fread(answer->fragment.data, 1, FRAGMENT_SIZE, f);

				answer->fragment.c = 60;
				answer->fragment.size = r + 4;
				answer->fragment.max_index = answer->fragment.index + r;

				msg_out("GET-CLIENT/ACK", "%s:%d",
					address_schar(net->current->v6.sin6_family, &net->current->v6.sin6_addr),
					ntohs(net->current->v6.sin6_port));

				net_write(net, answer, sizeof(*answer) + answer->fragment.size, 0);

				if (r < FRAGMENT_SIZE) {
					break;
				}

				answer->fragment.index += r;
			}
		}
	}

	r->type = REQUEST_GET_CLIENT_ACK;

	return;
}

void
check_file_completion(RegisteredFile* rf)
{
	unsigned char* hash;
	int i;

	vec_foreach (&rf->hash_data->chunkDigests, hash, i) {
		int* list = rf->received_fragments.data[i];

		for (int k = 0; k < 1000; k++) {
			if (list[k] == 0)
				return;
		}
	}

	printf("Whole file downloaded?\n");

	rf->status = STATUS_PUT;
	rf->timeout = 1;
}

void
check_chunk_completion(RegisteredFile* rf, int index, int* fragmentsList)
{
	int i;
	char buffer[CHUNK_SIZE];
	unsigned char hash[32];
	size_t size_read;

	for (i = 0; i < 1000; i++)
		if (fragmentsList[i] == 0)
			return;

	FILE* file = fopen(rf->filename, "r");

	fseek(file, index * CHUNK_SIZE, SEEK_SET);
	size_read = fread(buffer, 1, sizeof(buffer), file);
	fclose(file);

	sha256_hash(hash, (void*) buffer, size_read);

	if (!memcmp(hash, rf->hash_data->chunkDigests.data[index], 32)) {
		printf("Got a complete, valid chunk.\n");
	} else {
		orz("Chunk is broken, marking for re-download.");

		for (int k = 0; k < 1000; k++) {
			fragmentsList[k] = 0;
		}

		return;
	}

	check_file_completion(rf);
}

void
handle_get_client_ack(struct net* net, char* buffer, int count, vec_void_t* registered_files)
{
	RequestGetClientAck* r = (void*) buffer;
	RegisteredFile* rf;
	int i;

	msg_in("GET-CLIENT/ACK", "%s:%d",
		address_schar(net->addr.v6.sin6_family, &net->addr.v6.sin6_addr),
		ntohs(net->addr.v6.sin6_port));

	printf("%d -> %d:\n",
		r->fragment.index, r->fragment.max_index);

	vec_foreach (registered_files, rf, i) {
		if (!memcmp(rf->hash_data->digest, r->file_hash_segment.hash, 32)) {
			unsigned char* hash;
			int j;
			int foundChunk = 0;

			vec_foreach (&rf->hash_data->chunkDigests, hash, j) {
				if (!memcmp(hash, r->chunk_hash_segment.hash, 32)) {
					FILE* file;
					int index = r->fragment.index / FRAGMENT_SIZE;
					int* fragmentsList = rf->received_fragments.data[i];

					fragmentsList[index] = 1;
					if (r->fragment.max_index - r->fragment.index < FRAGMENT_SIZE) {
						for (int k = index + 1; k < 1000; k++) {
							fragmentsList[k] = 1;
						}
					}

					file = fopen(rf->filename, "r+");
					if (file == NULL)
						file = fopen(rf->filename, "w");

					fseek(file, i * CHUNK_SIZE + r->fragment.index, SEEK_SET);

					fwrite(r->fragment.data, 1, r->fragment.max_index - r->fragment.index, file);

					fclose(file);

					check_chunk_completion(rf, i, fragmentsList);

					foundChunk = 1;
					break;
				}
			}

			if (!foundChunk)
				orz("Received broken fragment?");

			break;
		}
	}
}

int
event_loop(struct net* net, struct net* srv, vec_void_t* registered_files)
{
	char buffer[CHUNK_SIZE * 2];

	vec_void_t connected_clients;

	struct timeval t;
	t.tv_sec  = 0;
	t.tv_usec = 0;

	vec_init(&connected_clients);

	net_set_timeout(net, &t);
	net_set_timeout(srv, &t);

	for (int i = 0;; i++)
	{
		struct net* active_net;
		int count;

		count = net_read_vec(net, srv, &connected_clients, &active_net, buffer, sizeof(buffer), 0);

		if (count < 0) {
			orz("something bad happened");
			net_error(count);
		} else if (count == 0) { /* timeout */
			handle_timeout(net, srv, registered_files, &connected_clients);

			t.tv_sec = 1;
		} else {
			RequestType type = buffer[0];

			switch (type) {
				case REQUEST_EC:
					handle_ec(buffer, count);
					break;
				case REQUEST_PUT_ACK:
					handle_put_ack(buffer, count, registered_files);
					break;
				case REQUEST_PUT_ERROR:
					handle_put_error(net, buffer, count, registered_files);
					break;
				case REQUEST_KEEP_ALIVE_ACK:
					handle_keep_alive_ack(buffer, count, registered_files);
					break;
				case REQUEST_KEEP_ALIVE_ERROR:
					handle_keep_alive_error(net, srv, buffer, count, registered_files);
					break;
				case REQUEST_GET_ACK:
					handle_get_ack(net, buffer, count, registered_files, &connected_clients);
					break;
				case REQUEST_LIST:
					handle_list(active_net, buffer, registered_files, srv);
					break;
				case REQUEST_LIST_ACK:
					handle_list_ack(active_net, buffer, registered_files );
					break;
				case REQUEST_GET_CLIENT:
					handle_get_client(active_net, buffer, count, registered_files);
					break;
				case REQUEST_GET_CLIENT_ACK:
					handle_get_client_ack(active_net, buffer, count, registered_files);
					break;
				default:
					orz("Unknown request type [%d]", type);
					break;
			}
		}
	}

	int i;
	struct net* client;
	vec_foreach (&connected_clients, client, i) {
		free(client);
	}
	vec_deinit(&connected_clients);

	return 0;
}

void
put_file(vec_void_t* files, const char* filename)
{
	RegisteredFile* rf = malloc(sizeof(*rf));

	strncpy(rf->filename, filename, PATH_MAX - 1);

	rf->status = STATUS_PUT;

	rf->timeout = 1;

	rf->hash_data = hash_data_new(filename);

	vec_init(&rf->received_fragments);

	vec_push(files, rf);

	msg_out( "PROCESSING", "Preparing to send file >");
	msg_out( "PROCESSING", "%s", filename );
	msg_file_hash_out( hash_data_schar(rf->hash_data->digest) );
}

void
get_file(vec_void_t* files, const char* digest, const char* filename)
{
	RegisteredFile* rf = malloc(sizeof(*rf));

	strncpy(rf->filename, filename, PATH_MAX - 1);

	rf->status = STATUS_GET;

	rf->timeout = 1;

	rf->hash_data = malloc(sizeof(HashData));
	rf->hash_data->filename = strdup(filename);

	rf->hash_data->digest = malloc(32);
	memset(rf->hash_data->digest, 0, 32);
	for (int i = 0; i < 32; i++) {
		if (digest[i] == '\0')
			break;

		unsigned int digit;

		sscanf(digest + 2 * i, "%02x", &digit);

		rf->hash_data->digest[i] = digit;
	}

	vec_init(&rf->hash_data->chunkDigests);

	vec_init(&rf->related_clients);
	vec_init(&rf->received_fragments);

	vec_push(files, rf);

	/* Maybe ? */
	msg_in("GET", "Preparing to receive file > %s", filename);
}

/**
 * Hand-made parameters parsing.
 *
 * Read the man page to know what it parses or not.
 *
 * Short option syntax is supported, but won’t be of use considering that all
 * possible options require a parameter.
 *
 * TODO: Don’t hardcode stuff!
 */
int
parse_arg(
	int argc, const char* argv[],
	uint16_t* port, uint16_t* listening_port,
	const char** destination,
	vec_void_t* registered_files
)
{
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (!strcmp(argv[i], "--put")) {
					if ((i + 1) < argc) {
						put_file(registered_files, argv[++i]);
					} else {
						orz("--put expects a file name");

						return 1;
					}
				} else if (!strcmp(argv[i], "--get")) {
					if ((i + 2) < argc) {
						get_file(registered_files, argv[i+1], argv[i+2]);

						i += 2;
					} else {
						orz("--get expects a hash and a file name");

						return 1;
					}
				} else if (!strcmp(argv[i], "--port")) {
					if ((i + 1) < argc) {
						(*port) = atol(argv[i+1]);

						i++;
					} else {
						orz("--port expects a port number");

						return 1;
					}
				} else if (!strcmp(argv[i], "--listening-port")) {
					if ((i + 1) < argc) {
						(*listening_port) = atol(argv[i+1]);

						i++;
					} else {
						orz("--listening-port expects a port number");

						return 1;
					}
				} else if (!strcmp(argv[i], "--host")) {
					if ((i + 1) < argc) {
						(*destination) = argv[i+1];

						i++;
					} else {
						orz("--host expects a hostname or address");

						return 1;
					}
				}
			} else {
				for (int j = 1; argv[i][j]; j++) {
					if (argv[i][j] == 'p') {
						if (argv[i][j + 1] == '\0' && (i + 1) < argc) {
							(*port) = atol(argv[i+1]);

							i++;

							break;
						} else {
							orz("-p expects a port number");

							return 1;
						}
					} else if (argv[i][j] == 'l') {
						if (argv[i][j + 1] == '\0' && (i + 1) < argc) {
							(*listening_port) = atol(argv[i+1]);

							i++;

							break;
						} else {
							orz("-l expects a port number");

							return 1;
						}
					} else if (argv[i][j] == 'h') {
						if (argv[i][j + 1] == '\0' && (i + 1) < argc) {
							(*destination) = argv[i+1];

							i++;

							break;
						} else {
							orz("-h expects a hostname or address");

							return 1;
						}
					}
				}
			}
		} else {
			orz("unexpected extra operand: %s", argv[i]);
			return 1;
		}
	}

	return 0;
}

int
main ( int argc, const char* argv[] )
{
	srand(time(NULL));
	uint16_t tracker_port = 9000;
	uint16_t peers_port = 9001;
	const char* destination = NULL;

	vec_void_t registered_files;

	vec_init(&registered_files);

	if (0 != parse_arg(argc, argv, &tracker_port, &peers_port, &destination, &registered_files))
		return 1;

	/* TODO: Is a default destination required? */
	if (!destination) {
		orz("no destination set, use -h <addr>");

		return 1;
	}

	/* `net` will be the connection to the tracker,
	 * whereas `srv` will be the listening socket for incoming connections from
	 * other peers */
	struct net net;
	struct net srv;

	int err = net_init(&net, tracker_port, destination, NET_CLIENT, NET_IPV4);
	net_error(err);
	err = net_init(&srv, peers_port, "0.0.0.0", NET_SERVER, NET_IPV4);
	net_error(err);

	event_loop(&net, &srv, &registered_files);

	/* Memory freeing, obviously. */
	for (int i = 0; i < registered_files.length; i++) {
		RegisteredFile* rf = registered_files.data[i];

		hash_data_free(rf->hash_data);

		for (int j = 0; j < rf->related_clients.length; j++)
			free(rf->related_clients.data[j]);

		for (int j = 0; j < rf->received_fragments.length; j++)
			free(rf->related_clients.data[j]);

		free(registered_files.data[i]);
	}
	vec_deinit(&registered_files);

	return 0;
}

