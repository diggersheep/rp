#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

