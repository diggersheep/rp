#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "net.h"

int
main(int argc, const char** argv)
{
	struct net net;
	size_t count;
	char buffer[1000000];

	(void) argc;
	(void) argv;

	net_server(&net, 9000, "::", NET_IPV6);

	printf("Tracker running on ::, port 9000.\n");

	while ((count = net_read(&net, buffer, sizeof(buffer), 0)) > 0) {
		RequestType type = buffer[0];

		switch (type) {
			case REQUEST_LIST:
				printf("Got a request to send a list of chunk hashes.\n");
				break;
			case REQUEST_GET:
				printf("Got a request to send a complete chunk.\n");
				break;
			case REQUEST_PRINT:
				puts("Got a print request.\n");
				break;
			case REQUEST_GET_ANSWER:
			case REQUEST_LIST_ANSWER:
				printf("Got an unhandled request [type=%u].\n", type);
				break;
		}
	}

	// net_shutdown();

	return 0;
}

