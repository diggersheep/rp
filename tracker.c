#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
#include "hash.h"
#include "net.h"
#include "debug.h"
#include "tracker.h"

bool
check_segment_file_hash(SegmentFileHash* s)
{
	if (s->c != 50) {
		orz("Received invalid file hash segment (c != 50).");

		return false;
	}

	if (s->size != 32) {
		orz("Received invalid file hash segment (size != 32).");

		return false;
	}

	return true;
}

bool
check_segment_client(SegmentClient* s)
{
	if (s->v4.c != 55) {
		orz("Received invalid client segment (c != 55).");

		return false;
	}

	if (s->v4.ipv != 6 && s->v4.ipv != 18) {
		orz("Received invalid client segment (ipv != 6 && ipv != 18).");

		return false;
	}

	return true;
}

int
register_hash(unsigned char* hash, struct net* net, vec_void_t* registered_hashes, int port, int keepalive)
{
	int i;
	RegisteredHash* rh;

	vec_foreach (registered_hashes, rh, i) {
		struct sockaddr_in* saddr = (void*) &rh->client;
		int sameHash = memcmp(rh->hash, hash, sizeof(rh->hash)) == 0;
		int sameHost = (saddr->sin_family == net->current->v4.sin_family) &&
			(0 == memcmp(&saddr->sin_addr, &net->current->v4.sin_addr, saddr->sin_family == AF_INET ? 4 : 16));

		if (!keepalive)
			sameHost = sameHost && (saddr->sin_port == port);

		if (sameHash && sameHost) {
			rh->time_to_live = 60;

			return 1;

			break;
		}
	}

	return 0;
}

void
handle_put(struct net* net, void* buffer, vec_void_t* registered_hashes, int keepalive)
{
	RequestPut* datagram = (void*) buffer;
	int hashExists = 0;

	int r = check_segment_file_hash(&datagram->hash_segment);

	if (!keepalive)
		r = r && check_segment_client(&datagram->client_segment);

	if (!r) {
		orz("Dropping PUT request.");

		return;
	}

	struct sockaddr_in* in = (void*) net->current;
	char address[INET6_ADDRSTRLEN];
	inet_ntop(net->current->v4.sin_family, &in->sin_addr, address, sizeof(address));

	msg_in(
		keepalive ? "KEEP-ALIVE" : "PUT",
		"%s:%d",
		address, ntohs(net->current->v4.sin_port)
	);

	hashExists = register_hash(datagram->hash_segment.hash, net, registered_hashes, datagram->client_segment.v4.port, keepalive);

	if (!hashExists) {
		if (keepalive) {
			RequestKeepAliveError* answer = buffer;

			orz("file is unregistered, sending KEEP-ALIVE/ERROR");

			answer->type = REQUEST_KEEP_ALIVE_ERROR;

			msg_out(
				"KEEP-ALIVE/ERR", "%s:%d", address, ntohs(net->current->v4.sin_port)
			);

			net_write(net, buffer, sizeof(*answer), 0);
		} else {
			RegisteredHash* rh;

			rh = malloc(sizeof(*rh));

			rh->time_to_live = 60;
			memcpy(rh->hash, datagram->hash_segment.hash, sizeof(rh->hash));
			memcpy(&rh->client, net->current, sizeof(rh->client));
			((struct sockaddr_in*) &rh->client)->sin_port = datagram->client_segment.v4.port;

			vec_push(registered_hashes, rh);

			msg_in("", "new file registered");

			datagram->type = REQUEST_PUT_ACK;

			msg_out(
				"PUT/ACK", "%s:%d", address, ntohs(net->current->v4.sin_port)
			);

			/* PUT and PUT/ACK are the same exact datagrams. */
			net_write(net, buffer, sizeof(RequestPut), 0);
		}
	} else {
		if (keepalive) {
			datagram->type = REQUEST_KEEP_ALIVE_ACK;

			msg_out(
				"KEEP-ALIVE/ACK", "%s:%d", address, ntohs(net->current->v4.sin_port)
			);

			/* Almost the same request types. The first fields are the exact same ones. */
			net_write(net, buffer, sizeof(RequestKeepAliveAck), 0);
		} else {
			RequestPutError* answer = buffer;

			msg_out(
				"PUT/ERR", "%s:%d", address, ntohs(net->current->v4.sin_port)
			);

			answer->type = REQUEST_PUT_ERROR;

			net_write(net, buffer, sizeof(*answer), 0);
		}
	}
}

void
handle_get(struct net* net, void* buffer, vec_void_t* registered_hashes)
{
	RequestGet* datagram = (void*) buffer;
	char answer_buffer[CHUNK_SIZE + sizeof(RequestGetAck)];
	RequestGetAck* answer = (void*) answer_buffer;
	char* currentClient = (void*) answer->clients;
	size_t datagram_size = sizeof(RequestGetAck);

	int r =
		check_segment_file_hash(&datagram->hash_segment) &&
		check_segment_client(&datagram->client_segment);

	if (!r) {
		orz("Dropping GET request.");

		return;
	}

	if (1) {
		struct sockaddr_in* in = (void*) net->current;
		char address[INET6_ADDRSTRLEN];
		inet_ntop(net->current->v4.sin_family, &in->sin_addr, address, sizeof(address));

		msg_in(
			"GET",
			"%s:%d",
			address, ntohs(net->current->v4.sin_port)
		);
	}

	answer->type = REQUEST_GET_ACK;
	memcpy(answer->hash_segment.hash, datagram->hash_segment.hash, sizeof(answer->hash_segment.hash));
	answer->count = 0;

	int i;
	RegisteredHash* rh;
	char address[INET6_ADDRSTRLEN];

	vec_foreach (registered_hashes, rh, i) {
		if (!memcmp(rh->hash, datagram->hash_segment.hash, sizeof(*rh->hash))) {
			struct sockaddr_in* saddr = (void*) &rh->client;
			inet_ntop(rh->client.sa_family, &saddr->sin_addr, address, sizeof(address));

			if (rh->client.sa_family == AF_INET) {
				SegmentClient4* client = (void*) currentClient;
				struct sockaddr_in* in = (void*) &rh->client;

				client->c = 55;
				client->ipv = 6;
				client->port = in->sin_port;

				memcpy(&client->address, &in->sin_addr, sizeof(client->address));

				
				msg_out("", "sending peer: %s : %d", address, ntohs(in->sin_port));

				answer->count += 1;
				currentClient += sizeof(SegmentClient4);
				datagram_size += sizeof(SegmentClient4);
			} else if (rh->client.sa_family == AF_INET6) {
				SegmentClient6* client = (void*) currentClient;
				struct sockaddr_in* in = (void*) &rh->client;

				client->c = 55;
				client->ipv = 18;
				client->port = in->sin_port;

				memcpy(&client->address, &in->sin_addr, sizeof(client->address));

				srsly(" - sending pair: %s:%d - ", address, ntohs(in->sin_port));

				answer->count += 1;
				currentClient += sizeof(SegmentClient6);
				datagram_size += sizeof(SegmentClient6);
			} else {
				orz("Unexpected protocol in registered client (%s, sa_family=%d)",
					address, rh->client.sa_family);
			}
		}
	}

	msg_out(
		"GET/ACK",
		"%s:%d",
		address, ntohs(net->current->v4.sin_port)
	);

	net_write(net, answer, datagram_size, 0);

	register_hash(datagram->hash_segment.hash, net, registered_hashes, net->current->v4.sin_port, 0);
}

void
handle_timeout(vec_void_t* registered_hashes)
{
	int i;
	RegisteredHash* rh;
	struct net net;

	char buffer[sizeof(RequestEC) + sizeof(SegmentFileHash)];
	RequestEC* ec = (void*) buffer;
	SegmentFileHash* hash_segment;

	ec->type = REQUEST_EC;
	ec->subtype = REQUEST_EC_TIMEOUT;
	ec->size = sizeof(SegmentFileHash);

	vec_foreach_rev(registered_hashes, rh, i) {
		if (--rh->time_to_live <= 0) {
			struct sockaddr_in* saddr = (void*) &rh->client;
			char buffer[128];

			inet_ntop(saddr->sin_family, (void*) &saddr->sin_addr, buffer, 128);

			if (saddr->sin_family == AF_INET) {
				net_client(&net, saddr->sin_port, buffer, saddr->sin_family);
			} else {
				net_client(&net, saddr->sin_port, buffer, saddr->sin_family);
			}

			hash_segment = (void*) ec->data;
			hash_segment->c = 50;
			hash_segment->size = 32;
			memcpy(&hash_segment->hash, rh->hash, 32);

			net_write(&net, (void*) &ec, sizeof(RequestEC) + sizeof(SegmentFileHash), 0);

			net_shutdown(&net);

			if (i != registered_hashes->length - 1) {
				registered_hashes->data[i] = registered_hashes->data[registered_hashes->length-1];
			}

			vec_pop(registered_hashes);

			wtf(" - hash expired - ");
		}
	}
}

int
parse_arg(int argc, const char** argv, uint16_t* port, const char** address)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !(strcmp(argv[i], "--help"))) {
			printf("usage: %s [-p port] [-a address]\n", argv[0]);
			return -1;
		} else if (!strcmp(argv[i], "-p")) {
			if (argc - i > 0) {
				*port = atol(argv[i+1]);

				i++;
			} else {
				return 1;
			}
		} else if (!strcmp(argv[i], "-a")) {
			if (argc - i > 0) {
				*address = argv[i+1];

				i++;
			} else {
				return 1;
			}
		} else {
			return 1;
		}
	}

	return 0;
}

int
main(int argc, const char** argv)
{
	struct net net;
	int count;
	uint16_t port = 9000;
	const char* address = "0.0.0.0";

	switch (parse_arg(argc, argv, &port, &address)) {
		case 0:
			break;
		case 1:
			return 1;
		case -1:
			return 0;
		default:
			return 255;
	}

	vec_void_t registered_hashes;

	/* Buffer of maximum possible packet size. */
	char buffer[CHUNK_SIZE + sizeof(unsigned char) + sizeof(uint16_t)];

	struct timeval timeout = {1, 0};

	(void) argc;
	(void) argv;

	vec_init(&registered_hashes);

	if (net_server(&net, port, address, NET_IPV4) == NET_FAIL) {
		orz("Could not init server.");

		vec_deinit(&registered_hashes);

		return 1;
	}

	net_set_timeout(&net, &timeout);

	msg_out("RUN", "The tracker is running");

	while ((count = net_read(&net, buffer, sizeof(buffer), 0)) >= 0) {
		if (count == 0) {
			handle_timeout(&registered_hashes);

			timeout.tv_sec = 1;

			continue;
		}

		RequestType type = buffer[0];

		/* FIXME: For each case, assert() that count is more than the matching expected datagram size. */
		switch (type) {
			case REQUEST_PUT:
				handle_put(&net, buffer, &registered_hashes, 0);
				break;
			case REQUEST_KEEP_ALIVE:
				handle_put(&net, buffer, &registered_hashes, 1);
				break;
			case REQUEST_GET:
				handle_get(&net, buffer, &registered_hashes);
				break;
			case REQUEST_PRINT:
				if (1) {
					int i;
					RegisteredHash* rh;
					char address[INET6_ADDRSTRLEN];

					printf("Registered hashes: <<\n");

					vec_foreach (&registered_hashes, rh, i) {
						printf("  Hash[%i]: ", i);
						print_hash(rh->hash);

						inet_ntop(rh->client.sa_family, &rh->client, address, sizeof(address));
						printf(" <%s>, ttl=%d\n", address, rh->time_to_live);

						puts("\n");
					}

					puts(">>\n");
				}
				break;
			case REQUEST_LIST:
			case REQUEST_LIST_ACK:
			case REQUEST_GET_ACK:
			case REQUEST_PUT_ACK:
			case REQUEST_PUT_ERROR:
			case REQUEST_KEEP_ALIVE_ACK:
			case REQUEST_KEEP_ALIVE_ERROR:
				orz("Got an unhandled request (type = %u).", type);
				break;
			case REQUEST_EC:
				if (1) {
					RequestEC* r = (void*) buffer;

					if ( r->subtype == 0 )
					{
						printf("\033[01;34m EC/MSG           << \033[01;37m");
						fflush(stdout);
						write(0, (const char*) r->data, r->size);
						printf("\n");
					}
					else
					{
						wtf("Received unhandled EC type.");
					}
				}

				break;
		}
	}

	/* FIXME: Data accessed by pointer needs to be freed. */
	vec_deinit(&registered_hashes);

	net_shutdown(&net);

	return 0;
}

