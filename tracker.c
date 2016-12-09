#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "hash.h"
#include "net.h"
#include "tracker.h"

int
main(int argc, const char** argv)
{
	struct net net;
	size_t count;

	vec_void_t registered_hashes;

	/* Buffer of maximum possible packet size. */
	char buffer[CHUNK_SIZE + sizeof(unsigned char) + sizeof(uint16_t)];

	(void) argc;
	(void) argv;

	vec_init(&registered_hashes);

	net_server(&net, 9000, "::", NET_IPV6);

	printf("Tracker running on ::, port 9000.\n");

	while ((count = net_read(&net, buffer, sizeof(buffer), 0)) > 0) {
		RequestType type = buffer[0];

		/* FIXME: For each case, assert() that count is more than the matching expected datagram size. */
		switch (type) {
			case REQUEST_PUT:
				/* FIXME: Insufficient debug. */
				if (1) {
					RequestPut* datagram = (void*) buffer;
					int hashExists = 0;

					int i;
					RegisteredHash* rh;

					vec_foreach (&registered_hashes, rh, i) {
						int sameHash = memcmp(rh->hash, datagram->chunk_hash, sizeof(rh->hash)) == 0;
						int sameHost = memcmp(&rh->client, net.current, sizeof(*net.current));

						if (1) {
							char debug[INET6_ADDRSTRLEN];
							inet_ntop(AF_INET6, &rh->client, debug, sizeof(rh->client));
							printf("<%s>\n", debug);
						}

						if (sameHash && sameHost) {
							hashExists = 1;

							break;
						}
					}

					if (!hashExists) {
						RegisteredHash* rh;

						rh = malloc(sizeof(*rh));

						memcpy(rh->hash, datagram->chunk_hash, sizeof(rh->hash));
						memcpy(&rh->client, net.current, sizeof(rh->client));

						vec_push(&registered_hashes, rh);

						puts("chunk registered\n");
					} else {
						puts("hash was put but already registered\n");
					}
				}
				break;
			case REQUEST_GET:
				printf("Client sent GET request…\n");
				if (1) {
					RequestList* datagram = (void*) buffer;
					char answer_buffer[CHUNK_SIZE + sizeof(RequestListAnswer)];
					RequestListAnswer* answer = (void*) answer_buffer;
					size_t answer_size = sizeof(unsigned char);

					answer->type = REQUEST_LIST_ANSWER;

					int i;
					RegisteredHash* rh;
					char address[INET6_ADDRSTRLEN];

					/* Chunk size should probably be enough. */
					answer = malloc(sizeof(*answer) + CHUNK_SIZE);

					vec_foreach (&registered_hashes, rh, i) {
						if (!memcmp(rh->hash, datagram->file_hash, sizeof(*rh->hash))) {
							inet_ntop(AF_INET6, &rh->client, address, sizeof(rh->client));

							printf("  -> %s [", address);
							print_hash(rh->hash);
							puts("] ");

							inet_ntop(AF_INET6, &rh->client, address, sizeof(rh->client));

							if (rh->client.sa_family == AF_INET) {
								printf("%s", address);
							} else if (rh->client.sa_family == AF_INET6) {
								printf("%s", address);
							} else {
								printf("%s", address);
								fprintf(stderr, "ohshit() unimplemented.\n");
							}

							puts("\n");
						}
					}
				}
			case REQUEST_LIST:
				break;
				if (1) {
					RequestGet* datagram = (void*) buffer;

					puts("Client sent LIST request… ");
					print_hash(datagram->file_hash);
					puts(", ");
					print_hash(datagram->chunk_hash);
					puts("\n");
				}
				printf("Got a request to send a complete chunk.\n");
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

						inet_ntop(AF_INET6, &rh->client, address, sizeof(rh->client));
						printf("<%s>\n", address);

						puts("\n");
					}

					puts(">>\n");
				}
				break;
			case REQUEST_GET_ANSWER:
			case REQUEST_LIST_ANSWER:
			case REQUEST_PUT_ACK:
				printf("Got an unhandled request [type=%u].\n", type);
				break;
		}
	}


	/* FIXME: Data accessed by pointer needs to be freed. */
	vec_deinit(&registered_hashes);

	net_shutdown(&net);

	return 0;
}

