#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vec/vec.h"
#include "sha256/sha256.h"

#include "common.h"

#include "hash.h"

struct options {
	char colors;
};

void
usage(const char* progname)
{
	fprintf(stderr, "usage: %s filename\n", progname);
}

void
file_hashes(const char* filename, struct options* opt)
{
	HashData* hd;
	hd = hash_data_new(filename);

	if (opt->colors)
		printf("\033[01;37m");

	printf("File:      ");

	if (opt->colors)
		printf("\033[01;34m");

	print_hash(hd->digest);

	if (opt->colors)
		printf("\033[00m");

	printf("\n");

	for (int i = 0; i < hd->chunkDigests.length; i++) {
		int j;

		if (opt->colors)
			printf("\033[01;32m");

		j = printf("Chunk %i: ", i);
		for (; j < 11; j++)
			printf(" ");

		if (opt->colors)
			printf("\033[01;34m");

		print_hash(hd->chunkDigests.data[i]);
		printf("\n");

		if (opt->colors)
			printf("\033[00m");
	}

	hash_data_free(hd);
}

int
main(int argc, const char** argv)
{
	int i;
	struct options opt;

	memset(&opt, 0, sizeof(opt));

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == '-') {
				if (!strcmp(argv[i], "--color")) {
					opt.colors = 1;
				} else {
					usage(argv[0]);
					exit(1);
				}
			} else {
				int j;

				for (j = 1; argv[i][j]; j++) {
					switch (argv[i][j]) {
						case 'c':
							opt.colors = 1;
							break;
						default:
							usage(argv[0]);
							exit(1);
					}
				}
			}
		} else {
			file_hashes(argv[i], &opt);
		}
	}
}

