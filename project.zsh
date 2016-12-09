
package="rp"
version="0.0.1"

targets=(hash-file server)
type[hash-file]=binary
sources[hash-file]="hash-file.c hash.c sha256/sha256.c vec/vec.c"

type[server]=binary
sources[server]="net.c hash.c sha256/sha256.c vec/vec.c server.c"

CC=clang
CFLAGS="-std=gnu11 -Wall -Wextra -g -O3"

dist=(Makefile project.zsh)

