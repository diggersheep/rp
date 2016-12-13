
package="rp-tests"
version="0.0.1"

targets=(server client peer)

type[server]=binary
sources[server]="server.c ../net.c ../debug.c"

type[client]=binary
sources[client]="client.c ../net.c ../debug.c"

type[peer]=binary
sources[peer]="peer.c ../net.c ../debug.c"

CFLAGS="-std=gnu11 -Wall -Wextra -Werror"

