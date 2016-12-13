
#ifndef HASH_H
#define HASH_H

#include "common.h"

/* print a hash */
void       print_hash(unsigned char[32]);
/* return a printable string (reprensatation of hash) */
char*      hash_data_schar(const unsigned char*);
/* create a hash */
HashData*  hash_data_new(const char*);
/* destroy a structure full of hashs */
void       hash_data_free(HashData*);

#endif

