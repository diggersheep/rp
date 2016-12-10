
package="rp"
version="0.0.1"

targets=(hash-file tracker client)

type[hash-file]=binary
sources[hash-file]="hash-file.c hash.c sha256/sha256.c vec/vec.c debug.c"

type[tracker]=binary
sources[tracker]="net.c hash.c sha256/sha256.c vec/vec.c tracker.c debug.c"

type[client]=binary
sources[client]="net.c hash.c sha256/sha256.c vec/vec.c client.c debug.c"

CC=clang
CFLAGS="-std=gnu11 -Wall -Wextra -g -O3"

dist=(Makefile project.zsh)

