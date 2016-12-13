#include <stdlib.h>
#include <unistd.h>

#include "../net.h"

int
main(int argc, const char** argv)
{
	char buffer[1024];
	struct net client;
	struct net server;
	size_t count;
	struct timeval timeout;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	net_client(&client, 6666, "127.0.0.1", NET_IPV4);
	net_server(&server, 6666, "0.0.0.0", NET_IPV4);

	for (int i = 1; i < argc; i++) {
		net_write(&client, argv[i], strlen(argv[i]), 0);
	}

	net_set_timeout(&client, &timeout);
	net_set_timeout(&server, &timeout);

	while ((count = net_read2(&server, &client, buffer, sizeof(buffer), 0)) > 0) {
		write(1, buffer, count);
		putchar('\n');
	}

	net_shutdown(&client);
	net_shutdown(&server);
}

