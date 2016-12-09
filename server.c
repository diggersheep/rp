#include <stdlib.h>
#include <stdio.h>

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

	while ((count = net_read(&net, buffer, sizeof(buffer), 0)) > 0) {
		
	}

	// net_shutdown();

	return 0;
}

