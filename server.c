#include <stdlib.h>
#include <stdio.h>

#include "common.h"
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

		switch (type) {
			case REQUEST_PUT:
				/* FIXME: Insufficient debug. */
				if(1) {
					RequestPut* datagram = (void*) buffer;
					RegisteredHash* rh;

					rh = malloc(sizeof(*rh));

					memcpy(rh->hash, datagram->chunk_hash, sizeof(rh->hash));
					//rh->associated_client = net->current;

					vec_push(&registered_hashes, rh);

					printf("chunk registered\n");
				}
				break;
			case REQUEST_LIST:
				printf("Got a request to send a list of chunk hashes.\n");
				break;
			case REQUEST_GET:
				printf("Got a request to send a complete chunk.\n");
				break;
			case REQUEST_PRINT:
				printf("Got a print request.\n");
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

