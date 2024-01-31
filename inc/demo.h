#ifndef DEMO_H
#define DEMO_H

#include <stdio.h>

/* demoheader struct */
typedef struct
{
    unsigned char version;
    char netversion[64];
    char mapname[64];
    unsigned char mapsize[4];   // Big endian int
    unsigned char mapcrc[4];    // Big endian int
    char type[8];
    unsigned char length[4];    // Big endian int
    char timestamp[20];
} demoheader;

/* demotimeline struct, only contains the raw data */
typedef struct
{
    char data[260];
} demotimeline;

/* Read demo header from demofile into given demoheader */
/* Returns a positive number on success, negative on fail */
int readdemoheader(FILE *demofile, demoheader *demoheader);

/* Read demo timeline from demofile into given demoheader */
/* Returns a positive number on success, negative on fail */
/* Must have read the header from demofile beforehand */
int readdemotimeline(FILE *demofile, demotimeline *demotimeline);

#endif // DEMO_H