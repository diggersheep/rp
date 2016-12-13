
#ifndef DEBUG_H
#define DEBUG_H

int orz(  const char* restrict, ...);
int wtf(  const char* restrict, ...);
int srsly(const char* restrict, ...);

int msg_in(const char* packet, const char *restrict fmt, ...);
int msg_out(const char* packet, const char *restrict fmt, ...);

#endif

