
#ifndef HASH_H
#define HASH_H

#include "common.h"

void       print_hash(unsigned char[32]);
HashData*  hash_data_new(const char*);
void       hash_data_free(HashData*);

#endif

