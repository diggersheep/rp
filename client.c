#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "net.h"
#include "common.h"
#include "hash.h"
#include "debug.h"

#include "client.h"

#define CMD_PUT 1
#define CMD_GET 2
#define CMD_PRINT 3
#define CMD_LIST 4
#define CMD_DEBUG 99

int
send_put(struct net* net, const HashData * hd)
{
	int err = 0;
	char buffer[sizeof(RequestPut)];
	RequestPut* rq = (void*) buffer;

	rq->type = REQUEST_PUT;

	rq->hash_segment.c = 50;
	rq->hash_segment.size = 32;
	memcpy((void*) rq->hash_segment.hash, hd->digest, sizeof(rq->hash_segment.hash));

	rq->client_segment.v4.c = 55;
	if (net->version == 4)
		rq->client_segment.v4.ipv = 6;
	else
		rq->client_segment.v4.ipv = 18;

	struct sockaddr_in* saddr = (struct sockaddr_in*) &net->addr;
	rq->client_segment.v4.port = saddr->sin_port;

	memcpy(
		(void*) rq->client_segment.v6.address,
		&saddr->sin_addr,
		net->version == 4 ? 4 : 16
	);

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

	net_write(net, (void*) rq, sizeof(*rq), 0);

	return err;
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
	}
}

void
handle_timeout(struct net* tracker, vec_void_t* registered_files)
{
	int i;
	RegisteredFile* rf;

	vec_foreach(registered_files, rf, i) {
		if (--rf->timeout == 0) {
			switch (rf->status) {
				case STATUS_PUT:
					srsly("PUT> %s", hash_data_schar(rf->hash_data->digest));
					send_put(tracker, rf->hash_data);
					rf->timeout = 5;
					break;
				case STATUS_KEEP_ALIVE:
					orz("should be sending new PUT here");
					rf->status = STATUS_PUT;
					rf->timeout = 5;
					break;
				case STATUS_GET:
					orz("should be sending new GET here");
					rf->timeout = 5;
					break;
			}
		} else if (rf->status == STATUS_KEEP_ALIVE) {
			if (rf->timeout <= 30 && rf->timeout % 5 == 0) {
				srsly("KEEP-ALIVE> %s", hash_data_schar(rf->hash_data->digest));
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
		char c = '\n';
		write(1, r->data, r->size);
		write(1, &c, 1);
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
		orz("received broken PUT/ACK, datagram too short");

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
handle_keep_alive_ack(char* buffer, int size, vec_void_t* registered_files)
{
	RequestKeepAliveAck* r = (void*) buffer;
	int i;
	RegisteredFile* rf;

	if ((unsigned) size < sizeof(*r)) {
		orz("received broken PUT/ACK, datagram too short");

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

int
event_loop(struct net* net, struct net* srv, vec_void_t* registered_files)
{
	char buffer[CHUNK_SIZE * 2];

	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 0;

	net_set_timeout(net, &t);
	net_set_timeout(srv, &t);

	for (int i = 0;; i++)
	{
		int count;

		count = net_read2(net, srv, buffer, sizeof(buffer), 0);

		if (count < 0) {
			orz("something bad happened");
			net_error(count);
		} else if (count == 0) { /* timeout */
			handle_timeout(net, registered_files);
		} else {
			RequestType type = buffer[0];

			switch (type) {
				case REQUEST_EC:
					handle_ec(buffer, count);
					break;
				case REQUEST_PUT_ACK:
					handle_put_ack(buffer, count, registered_files);
					break;
				case REQUEST_KEEP_ALIVE_ACK:
					handle_keep_alive_ack(buffer, count, registered_files);
					break;
			}
		}

		t.tv_sec = 1;
	}

	return 0;
}

int
parse_arg(
	int argc, const char* argv[],
	const char** filename, uint16_t* port, const char** destination,
	int* command
)
{
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (!strcmp(argv[i], "--port")) {
					if ((i + 1) < argc) {
						(*port) = atol(argv[i+1]);

						i++;
					} else {
						orz("--port expects a port number");

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
			if (!(*command)) {
				if (!strcmp(argv[i], "put")) {
					(*command) = CMD_PUT;
				} else if (!strcmp(argv[i], "get")) {
					(*command) = CMD_GET;
				} else if (!strcmp(argv[i], "print")) {
					(*command) = CMD_PRINT;
				} else if (!strcmp(argv[i], "debug")) {
					(*command) = CMD_DEBUG;
				} else {
					orz("received unexpected (*command): %s", argv[i]);
					return 1;
				}
			} else if (!(*filename)) {
				(*filename) = argv[i];
			} else {
				orz("unexpected extra operand: %s", argv[i]);
				return 1;
			}
		}
	}

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

	vec_push(files, rf);

	srsly("registered for PUT > %s", filename);
}

void
get_file(vec_void_t* files, const char* filename)
{
	RegisteredFile* rf = malloc(sizeof(*rf));

	strncpy(rf->filename, filename, PATH_MAX - 1);

	rf->status = STATUS_PUT;

	rf->timeout = 1;

	rf->hash_data = malloc(sizeof(HashData));
	rf->hash_data->filename = NULL;

	/* FIXME: this needs to be assigned a real hash */
	memset(rf->hash_data->digest, 0, 32);

	vec_init(&rf->hash_data->chunkDigests);

	vec_push(files, rf);

	srsly("registered for GET > %s", filename);
}

int
main ( int argc, const char* argv[] )
{
	const char* filename = NULL;
	uint16_t tracker_port = 9000;
	uint16_t peers_port = 9001;
	const char* destination = NULL;
	int command = 0;

	vec_void_t registered_files;

	if (0 != parse_arg(argc, argv, &filename, &tracker_port, &destination, &command))
		return 1;

	if (!command) {
		orz("no command, use 'get', 'put', 'print', or RTFM");

		return 1;
	}

	if ((command == CMD_PUT || command == CMD_GET) && !filename) {
		orz("add a filename, please");

		return 1;
	}

	if (!destination) {
		orz("no destination set, use -h <addr>");

		return 1;
	}

	vec_init(&registered_files);

	struct net net;
	struct net srv;

	int err = net_init(&net, tracker_port, destination, NET_CLIENT, NET_IPV4);
	net_error(err);
	err = net_init(&srv, peers_port, "0.0.0.0", NET_SERVER, NET_IPV4);
	net_error(err);

	if (net.version == 4)
		srv.current = (union s_addr *) &(net.addr.v4);
	else
		srv.current = (union s_addr *) &(net.addr.v6);

	if (command == CMD_PUT) {
		put_file(&registered_files, filename);
	} else if (command == CMD_GET) {
		/* filename here is a hash */
		get_file(&registered_files, filename);
	}

	event_loop(&net, &srv, &registered_files);

	for (int i = 0; i < registered_files.length; i++) {
		/* FIXME: Not freeing ->hash_data… */
		free(registered_files.data[i]);
	}
	vec_deinit(&registered_files);

	return 0;
}

