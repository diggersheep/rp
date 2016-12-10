#include <stdio.h>
#include <string.h>

#include "net.h"
#include "common.h"
#include "hash.h"


int send_put      ( struct net * net, struct net * srv, const char * hash, int type );
int send_put_all  ( struct net * net, struct net * srv, const HashData * hd );
int send_put_file ( struct net * net, const HashData * hd);

