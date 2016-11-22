
package="test"
version="0.0.1"

targets=(hash-file)
type[hash-file]=binary
sources[hash-file]="hash-file.c hash.c sha256/sha256.c vec/vec.c"

CC=clang
CFLAGS="-std=gnu11 -Wall -Wextra -g -O2"

