#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vec/vec.h"
#include "sha256/sha256.h"

#include "common.h"

#include "hash.h"

void
usage(const char* progname)
{
	fprintf(stderr, "usage: %s filename\n", progname);
}

int
main(int argc, const char** argv)
{
	HashData* hd;

	if (argc != 2) {
		usage(argv[0]);
		exit(1);
	}

	hd = hash_data_new(argv[1]);

	printf("File:      ");
	print_hash(hd->digest);
	printf("\n");

	for (int i = 0; i < hd->chunkDigests.length; i++) {
		int j = printf("Chunk %i: ", i);
		for (; j < 11; j++)
			printf(" ");

		print_hash(hd->chunkDigests.data[i]);
		printf("\n");
	}

	hash_data_free(hd);
}

