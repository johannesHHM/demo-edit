#ifndef DEMO_H
#define DEMO_H

#include <stdio.h>

typedef enum
{
    DEMOTICK = 0,
    DEMOSNAP = 1,
    DEMOMESSAGE = 2,
    DEMODELTA = 3
} demotype;

/* demoheader struct */
typedef struct
{
    unsigned char version;
    char netversion[64];
    char mapname[64];
    unsigned char mapsize[4]; // Big endian int
    unsigned char mapcrc[4];  // Big endian int
    char type[8];
    unsigned char length[4]; // Big endian int
    char timestamp[20];
} demoheader;

/* demotimeline struct, only contains the raw data */
typedef struct
{
    char data[260];
} demotimeline;

/* demomap struct */
typedef struct
{
    unsigned char sha256[32];
    unsigned char *data;
} demomap;

typedef struct
{
    unsigned short type;
    unsigned short id;
    int numdata;
    int *data;
} demosnapitem;

/* Chunk types */
typedef struct
{
    int datasize;
    int numitems;
    int *offsets;
    demosnapitem *items;
} demosnap;

typedef struct
{
    char keyframe;
    char innline;
    int delta;
} demotick;

typedef struct
{
} demodelta;

typedef struct
{
} demomessage;

/* union for different chunks */
typedef union {
    demosnap *snap;
    demotick *tick;
    demodelta *delta;
    demomessage *message;
} chunkdata;

/* demochunk struct */
typedef struct
{
    demotype type;
    chunkdata data;
} demochunk;

/* demodata struct, stores all chunks */
typedef struct
{
    int numchunks;
    demochunk *chunks;
} demodata;

/* Read demo header from demofile into given demoheader */
/* Returns a positive number on success, negative on fail */
int readdemoheader(FILE *demofile, demoheader *header);

/* Read demo timeline from demofile into given demoheader */
/* Returns a positive number on success, negative on fail */
/* Must have read the header from demofile beforehand */
int readdemotimeline(FILE *demofile, demotimeline *timeline);

/* Read demo map from demofile into given demomap */
/* Returns a positive number on success, negative on fail */
/* Must have read the demoheader and demotimeline beforehand */
int readdemomap(FILE *demofile, demomap *map, int mapsize);

/* Reads a chunk from demofile into given chunk */
/* Returns a positive number on success, 0 on EOF, and negative number on error */
/* Will allocate memory if chunk is snap, delta or message */
int readdemochunk(FILE *demofile, demochunk *chunk, unsigned char version);

#endif // DEMO_H
