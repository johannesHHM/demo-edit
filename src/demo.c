#include "../inc/demo.h"
#include "../inc/huffman.h"
#include "../inc/pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_TICK_MASK 0b10000000
#define CHUNK_TICK_KEYFRAME_MASK 0b01000000
#define CHUNK_TICK_INLINE_MASK 0b00100000
#define CHUNK_NORMAL_TYPE_MASK 0b01100000
#define CHUNK_NORMAL_SIZE_MASK 0b00011111

const unsigned char mapmagic[] = {0x6b, 0xe6, 0xda, 0x4a, 0xce, 0xbd, 0x38, 0x0c,
                                  0x9b, 0x5b, 0x12, 0x89, 0xc8, 0x42, 0xd7, 0x80};

// TODO make version sensitive
int readdemoheader(FILE *fp, demoheader *dh)
{
    if (fscanf(fp, "TWDEMO%*c%c", &dh->version) != 1)
        return -1;
    if (fscanf(fp, "%64c", dh->netversion) != 1)
        return -2;
    if (fscanf(fp, "%64c", dh->mapname) != 1)
        return -3;
    if (fread(dh->mapsize, 1, 4, fp) != 4)
        return -4;
    if (fread(dh->mapcrc, 1, 4, fp) != 4)
        return -5;
    if (fscanf(fp, "%8c", dh->type) != 1)
        return -6;
    if (fread(dh->length, 1, 4, fp) != 4)
        return -7;
    if (fscanf(fp, "%20c", dh->timestamp) != 1)
        return -8;
    printf("version: %hhu\n", dh->version);
    printf("net_version: %s\n", dh->netversion);
    printf("mapname: %s\n", dh->mapname);
    printf("mapsize: %i\n", reverseint(dh->mapsize));
    printf("mapcrc: %i\n", reverseint(dh->mapcrc));
    printf("type: %s\n", dh->type);
    printf("length: %i\n", reverseint(dh->length));
    printf("timestamp: %s\n", dh->timestamp);
    return 1;
}

int readdemotimeline(FILE *fp, demotimeline *dt)
{
    if (fscanf(fp, "%260c", dt->data) != 1)
        return -1;
    return 1;
}

// TODO make version sensitive
int readdemomap(FILE *fp, demomap *dm, int mapsize)
{
    char magicbuf[sizeof(mapmagic)];
    fread(magicbuf, 1, sizeof(mapmagic), fp);

    if (memcmp(magicbuf, mapmagic, sizeof(mapmagic)) != 0)
        return -1;

    if (fread(dm->sha256, 1, 32, fp) != 32)
        return -2;

    if (fread(dm->data, 1, mapsize, fp) != mapsize)
        return -3;

    return 1;
}

int readdemotick(FILE *fp, char chunkhead, demotick *tick, unsigned char ver)
{
    tick->keyframe = (chunkhead & CHUNK_TICK_KEYFRAME_MASK);

    if (ver >= 5)
    {
        if (chunkhead & CHUNK_TICK_INLINE_MASK)
        {
            tick->innline = 1;
            tick->delta = (chunkhead & 0x1f); // TODO name this mask?
        }
        else
        {
            tick->innline = 0;
            unsigned char tickdelta[4];
            fread(tickdelta, 1, 4, fp);
            tick->delta = reverseint(tickdelta);
        }
    }
    else
    {
        if ((chunkhead & 0x3f) != 0) // TODO name this mask?
            tick->delta = (chunkhead & 0x3f);
        else
        {
            unsigned char tickdelta[4];
            fread(tickdelta, 1, 4, fp);
        }
    }
    return 1;
}

int readdemosnap(FILE *fp, demosnap *snap, int size)
{
    unsigned char data[size];
    unsigned char unpacked[1024 * 8];

    fread(data, sizeof(unsigned char), size, fp);

    int r = decompresshuff((char *)data, size, (char *)unpacked, 1034 * 8);

    unsigned char *cp = unpacked;

    snap->datasize = readint(&cp);
    snap->numitems = readint(&cp);

    snap->offsets = (int *)malloc(snap->numitems * sizeof(int));
    snap->items = (demosnapitem *)malloc(snap->numitems * sizeof(int));

    for (int i = 0; i < snap->numitems; i++)
        snap->offsets[i] = readint(&cp);

    for (int i = 0; i < snap->numitems; i++)
    {
        unsigned int item_key = (unsigned int)readint(&cp);

        snap->items[i].type = (item_key >> 16) & 0xffff;
        snap->items[i].id = (item_key & 0xffff);

        if (i == (snap->numitems - 1))
            snap->items[i].numdata = (snap->datasize - snap->offsets[i]) / 4 - 1;
        else
            snap->items[i].numdata = (snap->offsets[i + 1] - snap->offsets[i]) / 4 - 1;

        snap->items[i].data = (int *)malloc(size * sizeof(int));

        for (int y = 0; y < snap->items[i].numdata; y++)
            snap->items[i].data[y] = readint(&cp);
    }

    return 1;
}

int readdemochunk(FILE *fp, demochunk *chunk, unsigned char ver)
{
    char chunkhead;
    chunkhead = (char)fgetc(fp);

    if (chunkhead == EOF)
        return 0;

    if (chunkhead & CHUNK_TICK_MASK)
    {
        demotick *tick = (demotick *)malloc(sizeof(demotick));
        if (readdemotick(fp, chunkhead, tick, ver))
        {
            chunk->type = 0;
            chunk->data.tick = tick;
            printf("TICK={keyframe: %d, innline: %d, delta: %d}\n", tick->keyframe, tick->innline, tick->delta);
        }
        else
        {
            printf("[ ERROR ] error while reading tick!\n");
            return -1;
        }
    }
    else
    {
        unsigned char type = (chunkhead & CHUNK_NORMAL_TYPE_MASK) >> 5;
        short size = (chunkhead & CHUNK_NORMAL_SIZE_MASK);

        if (size == 30)
            size = (short)fgetc(fp);
        else if (size == 31)
            fread(&size, sizeof(short), 1, fp);

        if (type == 1)
        {
            demosnap *snap = (demosnap *)malloc(sizeof(demosnap));
            if (readdemosnap(fp, snap, size))
            {
                chunk->type = 1; // TODO make a chunk-type enum
                chunk->data.snap = snap;

                printf("SNAPSHOT={datasize: %d, numitems: %d, offsets: [ ", snap->datasize, snap->numitems);
                for (int i = 0; i < snap->numitems; i++)
                    printf("%d, ", snap->offsets[i]);

                printf("], items: [ ");
                for (int i = 0; i < snap->numitems; i++)
                {
                    printf("(type: %d, id: %d) { ", snap->items[i].type, snap->items[i].id);
                    for (int y = 0; y < snap->items[i].numdata; y++)
                        printf("%d, ", snap->items[i].data[y]);
                    printf("} ");
                }
                printf("]\n");
            }
            else
            {
                printf("[ ERROR ] error while reading snapshot!\n");
                return -1;
            }
        }
        else if (type == 2)
        {
            printf("MESSAGE={");
            fseek(fp, size, SEEK_CUR);
        }
        else if (type == 3)
        {
            printf("SNAPSHOT_DELTA={");
            fseek(fp, size, SEEK_CUR);
        }

        printf("size: %d}\n", size);
    }
    return 1;
}
