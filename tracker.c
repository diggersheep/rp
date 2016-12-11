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

void
handle_put(struct net* net, void* buffer, vec_void_t* registered_hashes, int keepalive)
{
	RequestPut* datagram = (void*) buffer;
	int hashExists = 0;

	if (1) {
		printf("<");
		print_hash(datagram->hash_segment.hash);
		printf(">");

		struct sockaddr_in* in = (void*) net->current;
		char address[INET6_ADDRSTRLEN];
		inet_ntop(net->current->v4.sin_family, &in->sin_addr, address, sizeof(address));
		printf(" from %s\n", address);
	}

	int r = check_segment_file_hash(&datagram->hash_segment);

	if (!keepalive)
		r = r && check_segment_client(&datagram->client_segment);

	if (!r) {
		orz("Dropping PUT request.");

		return;
	}

	int i;
	RegisteredHash* rh;

	vec_foreach (registered_hashes, rh, i) {
		int sameHash = memcmp(rh->hash, datagram->hash_segment.hash, sizeof(rh->hash)) == 0;
		int sameHost = memcmp(&rh->client, net->current, sizeof(*net->current));

		if (sameHash && sameHost) {
			hashExists = 1;

			rh->time_to_live = 60;

			break;
		}
	}

	if (!hashExists) {
		if (keepalive) {
			RequestEC* answer = buffer;

			orz("received KEEP_ALIVE on unowned hash");

			answer->type = REQUEST_EC;
			answer->subtype = 0;

			strcpy((char*) answer->data, "can't keep an unregistered hash alive");

			answer->size = strlen((const char*) answer->data);

			net_write(net, buffer, sizeof(*answer) + answer->size, 0);
		} else {
			RegisteredHash* rh;

			rh = malloc(sizeof(*rh));

			rh->time_to_live = 60;
			memcpy(rh->hash, datagram->hash_segment.hash, sizeof(rh->hash));
			memcpy(&rh->client, net->current, sizeof(rh->client));

			vec_push(registered_hashes, rh);

			srsly("file registered");

			datagram->type = REQUEST_PUT_ACK;

			/* PUT and PUT/ACK are the same exact datagrams. */
			net_write(net, buffer, sizeof(RequestPut), 0);
		}
	} else {
		if (keepalive) {
			srsly("keeping file alive");

			datagram->type = REQUEST_KEEP_ALIVE_ACK;

			/* Almost the same request types. The first fields are the exact same. */
			net_write(net, buffer, sizeof(RequestKeepAliveAck), 0);
		} else {
			RequestEC* answer = buffer;

			wtf("hash was put but already registered");

			answer->type = REQUEST_EC;
			answer->subtype = 0;

			strcpy((char*) answer->data, "hash is already registered");

			answer->size = strlen((const char*) answer->data);

			net_write(net, buffer, sizeof(*answer) + answer->size, 0);
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

			srsly("GET/ACK> %s", hash_data_schar(rh->hash));

			if (rh->client.sa_family == AF_INET) {
				SegmentClient4* client = (void*) currentClient;
				struct sockaddr_in* in = (void*) &rh->client;

				client->c = 55;
				client->ipv = 6;
				client->port = in->sin_port;

				memcpy(&client->address, &in->sin_addr, sizeof(client->address));

				srsly("GET/ACK> - ipv4 - %s", address);

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

				srsly("GET/ACK> - ipv6 - %s", address);

				answer->count += 1;
				currentClient += sizeof(SegmentClient6);
				datagram_size += sizeof(SegmentClient6);
			} else {
				orz("Unexpected protocol in registered client (%s, sa_family=%d)",
					address, rh->client.sa_family);
			}
		}
	}

	wtf("GET/ACK> %d", datagram_size);
	net_write(net, answer, datagram_size, 0);
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

			wtf("hash expired");
		} else
			wtf("[ttl=%d]", rh->time_to_live);
	}
}

int
main(int argc, const char** argv)
{
	struct net net;
	int count;

	vec_void_t registered_hashes;

	/* Buffer of maximum possible packet size. */
	char buffer[CHUNK_SIZE + sizeof(unsigned char) + sizeof(uint16_t)];

	struct timeval timeout = {1, 0};

	(void) argc;
	(void) argv;

	vec_init(&registered_hashes);

	if (net_server(&net, 9000, "0.0.0.0", NET_IPV4) == NET_FAIL) {
		orz("Could not init server.");

		vec_deinit(&registered_hashes);

		return 1;
	}

	net_set_timeout(&net, &timeout);

	while ((count = net_read(&net, buffer, sizeof(buffer), 0)) >= 0) {
		if (count == 0) {
			handle_timeout(&registered_hashes);

			timeout.tv_sec = 1;

			continue;
		}

		RequestType type = buffer[0];

		printf("%d: ", count);

		/* FIXME: For each case, assert() that count is more than the matching expected datagram size. */
		switch (type) {
			case REQUEST_PUT:
				handle_put(&net, buffer, &registered_hashes, 0);
				break;
			case REQUEST_KEEP_ALIVE:
				handle_put(&net, buffer, &registered_hashes, 1);
				break;
			case REQUEST_GET:
				printf("Client sent GET request…\n");
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
			case REQUEST_KEEP_ALIVE_ACK:
				orz("Got an unhandled request (type = %u).", type);
				break;
			case REQUEST_EC:
				if (1) {
					RequestEC* r = (void*) buffer;

					write(0, (const char*) r->data, r->size);
					putchar('\n');
				}

				break;
		}
	}

	/* FIXME: Data accessed by pointer needs to be freed. */
	vec_deinit(&registered_hashes);

	net_shutdown(&net);

	return 0;
}

