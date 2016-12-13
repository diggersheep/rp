#include <stdlib.h>
#include <unistd.h>

#include "../net.h"

int
main()
{
	char buffer[1024];
	struct net server;
	size_t count;
	struct timeval timeout;

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	net_server(&server, 6666, "0.0.0.0", NET_IPV4);

	net_set_timeout(&server, &timeout);

	while ((count = net_read(&server, buffer, sizeof(buffer), 0)) > 0) {
		write(1, buffer, count);
		putchar('\n');
	}

	net_shutdown(&server);
}

