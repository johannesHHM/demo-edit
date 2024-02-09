#ifndef PACK_H
#define PACK_H

#include <stdio.h>

/* turns a big endian int into an int */
int frombigendian(const unsigned char in[4]);

/* writes an int into a big endian int in given buffer */
void tobigendian(const int in, unsigned char out[4]);

/* reads a teeworlds packed int from given buffer */
/* the given pointer is incremented */
int readint(char **cp);

/* writes given int to teeworlds packed */
/* the given pointer is incremented */
void writeint(int in, char **cp);

/* reads a teworlds string from given ints */
void intstostr(const int *ints, int num, char *str);

/* writes a teeworlds string into ints */
void strtoint(const char *str, int num, int *ints);

#endif // PACK_H
