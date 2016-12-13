#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>


#include "hash.h"

int
orz(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;

	fputs("\033[01;31m> ", stderr);

	va_start(ap, fmt);
	ret = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputs("\033[00m\n", stderr);

	return ret;
}

int
wtf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;

	fputs("\033[01;33m> ", stderr);

	va_start(ap, fmt);
	ret = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputs("\033[00m\n", stderr);

	return ret;
}

int
srsly(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;

	fputs("\033[01;34m> \033[01;37m", stdout);

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);

	fputs("\033[00m\n", stdout);

	return ret;
}

int
msg_in(const char* packet, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;

	fprintf(stdout, "\033[01;34m %-16s << \033[01;37m", packet);

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);

	fputs("\033[00m\n", stdout);

	return ret;
}

int
msg_out(const char* packet, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;

	fprintf(stdout, "\033[01;34m %-16s >> \033[01;37m", packet);

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);

	fputs("\033[00m\n", stdout);

	return ret;
}

void
msg_file_hash_out ( const char * hash )
{
	fprintf(stdout,
		"\033[01;34m    file hash     >> \033[01;35m%s\033[01;37m\n", hash );
}

void
msg_file_hash_in ( const char * hash )
{
	fprintf(stdout,
		"\033[01;34m    file hash     >> \033[01;35m%s\033[01;37m\n", hash );
}

void
msg_chunk_hash_out ( const char * hash, int index )
{
	fprintf(stdout,
		"\033[01;34m    chunk hash %02d << \033[01;35m%s\033[01;37m\n",
		index,
		hash
	);
}

void
msg_chunk_hash_in ( const char * hash, int index )
{
	fprintf(stdout,
		"\033[01;34m    chunk hash %02d << \033[01;35m%s\033[01;37m\n",
		index,
		hash
	);
}