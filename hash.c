#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vec/vec.h"
#include "sha256/sha256.h"

#include "common.h"

void
print_hash(unsigned char buffer[32])
{
	for (int i = 0; i < 32; i++)
		printf("%02x", buffer[i]);
}

HashData*
hash_data_new(const char* filename)
{
	sha256_t hash;
	unsigned char* digest;
	unsigned char buffer[CHUNK_SIZE];
	FILE* file;
	size_t count;
	HashData* hd;
	
	hd = malloc(sizeof(*hd));
	hd->filename = strdup(filename);
	vec_init(&hd->chunkDigests);

	file = fopen(filename, "r");
	if (!file) {
		perror("fopen");
		free(hd->filename);
		free(hd);
		return NULL;
	}

	sha256_init(&hash);

	int i = 0;
	while ((count = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
		unsigned char* chunkString;
		unsigned char bufferCopy[CHUNK_SIZE];

		chunkString = malloc(32);

		memcpy(bufferCopy, buffer, count);
		sha256_hash(chunkString, bufferCopy, count);

		sha256_update(&hash, buffer, count);

		vec_push(&hd->chunkDigests, (void*) chunkString);

		i++;
	}

	fclose(file);

	digest = malloc(32);
	sha256_final(&hash, digest);

	hd->digest = digest;

	return hd;
}

void
hash_data_free(HashData* hd)
{
	while (hd->chunkDigests.length > 0) {
		char* digest = vec_pop(&hd->chunkDigests);

		free(digest);
	}

	vec_deinit(&hd->chunkDigests);

	free(hd->filename);
	free(hd->digest);
	free(hd);
}

