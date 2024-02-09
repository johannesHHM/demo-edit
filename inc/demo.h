#ifndef DEMO_H
#define DEMO_H

#include <stdio.h>

typedef enum
{
    DEMOTICK = 0,
    DEMOSNAP = 1,
    DEMOMESSAGE = 2,
    DEMODELTA = 3
} demochunktype;

/* demoheader struct */
typedef struct
{
    unsigned char version;
    char netversion[64];
    char mapname[64];
    int mapsize;
    int mapcrc;
    char type[8];
    int length;
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
    char *data;
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
    int datasize;
    char *data;
} demodelta;

typedef struct
{
    int datasize;
    char *data;
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
    demochunktype type;
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
int readdemomap(FILE *demofile, demomap *map, int mapsize, unsigned char version);

/* Reads a chunk from demofile into given chunk */
/* Returns a positive number on success, 0 on EOF, and negative number on error */
/* Will allocate memory if chunk is snap, delta or message */
int readdemochunk(FILE *demofile, demochunk *chunk, unsigned char version);

int readdemochunks(FILE *fp, demodata *dd, unsigned char ver);

int writedemoheader(FILE *outfile, demoheader *header);
int writedemotimeline(FILE *outfile, demotimeline *timeline);
int writedemomap(FILE *outfile, demomap *map, int mapsize, unsigned char version);

int writedemotick(FILE *outfile, demotick *tick, unsigned char version);
int writedemosnap(FILE *outfile, demosnap *snap, unsigned char version);
int writedemomessage(FILE *outfile, demomessage *message, unsigned char ver);
int writedemodelta(FILE *outfile, demodelta *delta, unsigned char ver);
#endif // DEMO_H
