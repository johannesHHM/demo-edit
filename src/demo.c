/* This file is part of dedit - a tool for editing Teeworlds/DDNet demo files
   Copyright (C) 2024-2026 JHHM

   Dedit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Dedit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with dedit.  If not, see <https://www.gnu.org/licenses/>. */

#define _POSIX_C_SOURCE 200112L

#include "demo.h"
#include "huffman.h"
#include "pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_TICK_MASK 0x80          // 0b10000000
#define CHUNK_TICK_KEYFRAME_MASK 0x40 // 0b01000000
#define CHUNK_TICK_INLINE_MASK 0x20   // 0b00100000
#define CHUNK_TICK_DELTA_V3_MASK 0x3f // 0b00111111
#define CHUNK_TICK_DELTA_V5_MASK 0x1f // 0b00011111
#define CHUNK_NORMAL_TYPE_MASK 0x60   // 0b01100000
#define CHUNK_NORMAL_SIZE_MASK 0x1f   // 0b00011111

const char headermagic[7] = "TWDEMO";
const unsigned char mapmagic[] = {0x6b, 0xe6, 0xda, 0x4a, 0xce, 0xbd, 0x38, 0x0c,
                                  0x9b, 0x5b, 0x12, 0x89, 0xc8, 0x42, 0xd7, 0x80};

#define BUFF_SIZE 1024 * 64

int snapitemsizes06[] = {
    0,  // index 0, unused
    10, // id  1
    6,  // id  2
    5,  // id  3
    4,  // id  4
    3,  // id  5
    8,  // id  6
    4,  // id  7
    15, // id  8
    22, // id  9
    5,  // id 10
    17, // id 11
    3,  // id 12
    2,  // id 13
    2,  // id 14
    2,  // id 15
    2,  // id 16
    3,  // id 17
    3,  // id 18
    3,  // id 19
    3   // id 20
};

void freedemo(demo *demo)
{
    free(demo->map.data);

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        demochunk *chunk = &demo->data.chunks[i];
        switch (chunk->type)
        {
        case DEMOTICK:
            free(chunk->data.tick);
            break;
        case DEMOSNAP:
            for (int y = 0; y < chunk->data.snap->numitems; y++)
                free(chunk->data.snap->items[y].data);
            free(chunk->data.snap->offsets);
            free(chunk->data.snap->items);
            free(chunk->data.snap);
            break;
        case DEMOMESSAGE:
            free(chunk->data.message->data);
            free(chunk->data.message);
            break;
        case DEMODELTA:
            free(chunk->data.delta->removeditemkeys);
            for (int i = 0; i < chunk->data.delta->numitemdeltas; i++)
                free(chunk->data.delta->itemdeltas[i].data);
            free(chunk->data.delta->itemdeltas);
            break;
        default:
            break;
        }
    }
    free(demo->data.chunks);
}

/* readers */

int readdemoheader(FILE *fp, demoheader *dh)
{
    char magicbuff[sizeof(headermagic)];
    unsigned char intbuff[4];

    fread(magicbuff, 1, sizeof(headermagic), fp);
    if (strcmp(magicbuff, headermagic) != 0)
        return -1;

    if (fread(&dh->version, 1, 1, fp) != 1)
        return -2;

    if (fread(&dh->netversion, 1, 64, fp) != 64)
        return -3;

    if (fread(&dh->mapname, 1, 64, fp) != 64)
        return -4;

    if (fread(intbuff, 1, 4, fp) != 4)
        return -5;
    dh->mapsize = frombigendian(intbuff);

    if (fread(intbuff, 1, 4, fp) != 4)
        return -6;
    dh->mapcrc = frombigendian(intbuff);

    if (fread(dh->type, 1, 8, fp) != 8)
        return -7;

    if (fread(intbuff, 1, 4, fp) != 4)
        return -8;
    dh->length = frombigendian(intbuff);

    if (fread(dh->timestamp, 1, 20, fp) != 20)
        return -9;

    return 1;
}

int readdemotimeline(FILE *fp, demotimeline *dt)
{
    if (fread(dt->data, 1, DEMO_TIMELINE_LENGTH, fp) != DEMO_TIMELINE_LENGTH)
        return -1;
    return 1;
}

int readdemomap(FILE *fp, demomap *dm, int mapsize, unsigned char ver)
{
    if (ver >= 6)
    {
        char magicbuf[sizeof(mapmagic)];

        fread(magicbuf, 1, sizeof(mapmagic), fp);
        if (memcmp(magicbuf, mapmagic, sizeof(mapmagic)) != 0)
            return -1;

        if (fread(dm->sha256, 1, 32, fp) != 32)
            return -2;
    }

    dm->data = (char *)malloc(mapsize);
    if (fread(dm->data, 1, mapsize, fp) != (size_t)mapsize)
        return -3;

    return 1;
}

int readdemotick(FILE *fp, char chunkhead, demotick *tick, unsigned char ver)
{
    tick->keyframe = (chunkhead & CHUNK_TICK_KEYFRAME_MASK) >> 6;

    if (ver >= 5)
    {
        if (chunkhead & CHUNK_TICK_INLINE_MASK)
        {
            tick->innline = 1;
            tick->delta = (chunkhead & CHUNK_TICK_DELTA_V5_MASK);
        }
        else
        {
            tick->innline = 0;
            unsigned char tickdelta[4];
            fread(tickdelta, 1, 4, fp);
            tick->delta = frombigendian(tickdelta);
        }
    }
    else
    {
        if ((chunkhead & CHUNK_TICK_DELTA_V3_MASK) != 0)
        {
            tick->delta = (chunkhead & CHUNK_TICK_DELTA_V3_MASK);
            tick->innline = 1;
        }
        else
        {
            tick->innline = 0;
            unsigned char tickdelta[4];
            fread(tickdelta, 1, 4, fp);
            tick->delta = frombigendian(tickdelta);
        }
    }
    return 1;
}

int readdemosnap(FILE *fp, demosnap *snap, int size)
{
    char data[size];
    char unpacked[BUFF_SIZE];

    fread(data, 1, size, fp);

    if (decompresshuff((char *)data, size, (char *)unpacked, BUFF_SIZE) < 0)
    {
        printf("[ ERROR ] error while decompressing snap cunk!\n");
        return 0;
    }

    char *cp = unpacked;

    snap->datasize = readint(&cp);
    snap->numitems = readint(&cp);

    snap->offsets = (int *)malloc(snap->numitems * sizeof(int));
    snap->items = (demosnapitem *)malloc(snap->numitems * sizeof(demosnapitem));

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

        snap->items[i].data = (int *)malloc(snap->items[i].numdata * sizeof(int));

        for (int y = 0; y < snap->items[i].numdata; y++)
            snap->items[i].data[y] = readint(&cp);
    }

    return 1;
}

int readdemomessage(FILE *fp, demomessage *message, int size)
{
    unsigned char data[size];
    if (fread(data, 1, size, fp) != (size_t)size)
        return -1;

    message->data = (char *)malloc(size);
    memcpy(message->data, data, size);
    message->datasize = size;

    return 1;
}

int readdemodelta(FILE *fp, demodelta *delta, int size)
{
    char data[size];
    if (fread(data, 1, size, fp) != (size_t)size)
        return -1;

    char unpacked[BUFF_SIZE];

    int s = decompresshuff(data, size, unpacked, BUFF_SIZE);
    if (s < 0)
    {
        printf("[ ERROR ] error while decompressing delta cunk!\n");
        return 0;
    }

    char *cp = unpacked;
    delta->numremoveditems = readint(&cp);
    delta->numitemdeltas = readint(&cp);
    int zeros = readint(&cp);

    if (zeros != 0)
        printf("[ WARNING ] zeroes in delta is not zero!\n");

    if (delta->numremoveditems > 0)
        delta->removeditemkeys = (int *)malloc(delta->numremoveditems * sizeof(int));

    for (int i = 0; i < delta->numremoveditems; i++)
        delta->removeditemkeys[i] = readint(&cp);

    if (delta->numitemdeltas > 0)
        delta->itemdeltas = (demodeltaitem *)malloc(delta->numitemdeltas * sizeof(demodeltaitem));

    for (int i = 0; i < delta->numitemdeltas; i++)
    {
        demodeltaitem *item = &delta->itemdeltas[i];
        item->type = readint(&cp);
        item->id = readint(&cp);

        // TODO: 1 <= type <= 20 is 0.6 protocol spesific
        if (item->type >= 1 && item->type <= 20)
            item->size = snapitemsizes06[item->type];
        else
            item->size = readint(&cp);

        item->data = (int *)malloc(item->size * sizeof(int));

        for (int i = 0; i < item->size; i++)
            item->data[i] = readint(&cp);
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
        memset(tick, 0, sizeof(demotick)); // TODO idk if necessary

        if (readdemotick(fp, chunkhead, tick, ver))
        {
            chunk->type = DEMOTICK;
            chunk->data.tick = tick;
        }
        else
        {
            printf("[ ERROR ] error while reading tick!\n");
            return -1;
        }
    }
    else
    {
        demochunktype type = (demochunktype)((chunkhead & CHUNK_NORMAL_TYPE_MASK) >> 5);
        short size = (chunkhead & CHUNK_NORMAL_SIZE_MASK);

        if (size == 30)
            size = (short)fgetc(fp);
        else if (size == 31)
            fread(&size, sizeof(short), 1, fp);

        if (type == DEMOSNAP)
        {
            demosnap *snap = (demosnap *)malloc(sizeof(demosnap));
            memset(snap, 0, sizeof(demosnap)); // TODO idk if necessary

            if (readdemosnap(fp, snap, size))
            {
                chunk->type = DEMOSNAP;
                chunk->data.snap = snap;
            }
            else
            {
                printf("[ ERROR ] error while reading snapshot!\n");
                return -1;
            }
        }
        else if (type == DEMOMESSAGE)
        {
            demomessage *message = (demomessage *)malloc(sizeof(demomessage));
            readdemomessage(fp, message, size);
            chunk->type = DEMOMESSAGE;
            chunk->data.message = message;
        }
        else if (type == DEMODELTA)
        {
            demodelta *delta = (demodelta *)malloc(sizeof(demodelta));
            readdemodelta(fp, delta, size);
            chunk->type = DEMODELTA;
            chunk->data.delta = delta;
        }
        else
        {
            printf("[ ERROR ] unknown chunk type!\n");
        }
    }
    return 1;
}

int readdemochunks(FILE *fp, demodata *dd, unsigned char ver)
{
    int chunkcap = 1024;
    dd->numchunks = 0;
    dd->chunks = (demochunk *)malloc(chunkcap * sizeof(demochunk));

    while (readdemochunk(fp, &dd->chunks[dd->numchunks], ver))
    {
        dd->numchunks++;
        if (dd->numchunks >= chunkcap)
        {
            chunkcap *= 2;
            dd->chunks = (demochunk *)realloc(dd->chunks, chunkcap * sizeof(demochunk));
        }
    }

    dd->chunks = (demochunk *)realloc(dd->chunks, dd->numchunks * sizeof(demochunk));

    return 1;
}

int readdemo(FILE *fp, demo *demo)
{
    if (readdemoheader(fp, &demo->header) <= 0)
        return -1;

    if (readdemotimeline(fp, &demo->timeline) <= 0)
        return -2;

    if (readdemomap(fp, &demo->map, demo->header.mapsize, demo->header.version) <= 0)
        return -3;

    readdemochunks(fp, &demo->data, demo->header.version);

    return 1;
}

/* writers */

int writedemoheader(FILE *fp, demoheader *dh)
{
    unsigned char intbuff[4];
    if (fwrite(headermagic, 1, sizeof(headermagic), fp) != sizeof(headermagic))
        return -1;

    if (fwrite(&dh->version, 1, 1, fp) != 1)
        return -2;

    if (fwrite(dh->netversion, 1, 64, fp) != 64)
        return -3;

    if (fwrite(dh->mapname, 1, 64, fp) != 64)
        return -4;

    tobigendian(dh->mapsize, intbuff);
    if (fwrite(intbuff, 1, 4, fp) != 4)
        return -5;

    tobigendian(dh->mapcrc, intbuff);
    if (fwrite(intbuff, 1, 4, fp) != 4)
        return -5;

    if (fwrite(dh->type, 1, 8, fp) != 8)
        return -6;

    tobigendian(dh->length, intbuff);
    if (fwrite(intbuff, 1, 4, fp) != 4)
        return -7;

    if (fwrite(dh->timestamp, 1, 20, fp) != 20)
        return -8;

    return 1;
}

int writedemotimeline(FILE *fp, demotimeline *dt)
{
    if (fwrite(dt->data, 1, DEMO_TIMELINE_LENGTH, fp) != DEMO_TIMELINE_LENGTH)
        return -1;
    return 1;
}

int writedemomap(FILE *fp, demomap *dm, int mapsize, unsigned char ver)
{
    if (ver >= 6)
    {
        // TODO is this magic for v6+ only?
        if (fwrite(mapmagic, 1, sizeof(mapmagic), fp) != sizeof(mapmagic))
            return -1;
        if (fwrite(dm->sha256, 1, 32, fp) != 32)
            return -1;
    }

    if (fwrite(dm->data, 1, mapsize, fp) != (size_t)mapsize)
        return -2;

    return 1;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/79df5893ff26fa75d67e46f99e58f75b739ac362/src/engine/shared/demo.cpp#L270
*/
int writedemochunkheader(FILE *fp, demochunktype type, int size)
{
    unsigned char header[3];
    header[0] = (((int)type) & 0x3) << 5;
    if (size < 30)
    {
        header[0] |= size;
        fputc(*header, fp);
    }
    else
    {
        if (size < 256)
        {
            header[0] |= 30;
            header[1] = size & 0xff;
            fwrite(header, 1, 2, fp);
        }
        else
        {
            header[0] |= 31;
            header[1] = size & 0xff;
            header[2] = size >> 8;
            fwrite(header, 1, 3, fp);
        }
    }
    return -1;
}

// TODO add erro checking
int writedemotick(FILE *fp, demotick *tick, unsigned char ver)
{
    unsigned char header = CHUNK_TICK_MASK;
    header |= tick->keyframe << 6;
    if (ver >= 5)
        header |= tick->innline << 5;

    if (tick->innline)
    {
        header |= tick->delta;
        fputc(header, fp);
    }
    else
    {
        fputc(header, fp);
        unsigned char intbuf[4];
        tobigendian(tick->delta, intbuf);
        fwrite(intbuf, 4, 1, fp);
    }

    return 1;
}

int writedemosnap(FILE *fp, demosnap *snap)
{
    char compressed[BUFF_SIZE];
    char decompressed[BUFF_SIZE];

    char *cp = decompressed;

    writeint(snap->datasize, &cp);
    writeint(snap->numitems, &cp);

    for (int i = 0; i < snap->numitems; i++)
        writeint(snap->offsets[i], &cp);

    for (int i = 0; i < snap->numitems; i++)
    {
        unsigned int key = (snap->items[i].type << 16) | snap->items[i].id;
        writeint((int)key, &cp);

        for (int y = 0; y < snap->items[i].numdata; y++)
            writeint(snap->items[i].data[y], &cp);
    }
    int size = cp - decompressed;

    int datasize = compresshuff(decompressed, size, compressed, BUFF_SIZE);

    writedemochunkheader(fp, DEMOSNAP, datasize);
    fwrite(compressed, 1, datasize, fp);

    return 1;
}

int writedemomessage(FILE *fp, demomessage *message)
{
    writedemochunkheader(fp, DEMOMESSAGE, message->datasize);
    fwrite(message->data, 1, message->datasize, fp);

    return 1;
}

int writedemodelta(FILE *fp, demodelta *delta)
{
    char compressed[BUFF_SIZE];
    char decompressed[BUFF_SIZE];

    char *cp = decompressed;

    writeint(delta->numremoveditems, &cp);
    writeint(delta->numitemdeltas, &cp);
    writeint(0, &cp);

    for (int i = 0; i < delta->numremoveditems; i++)
        writeint(delta->removeditemkeys[i], &cp);

    for (int i = 0; i < delta->numitemdeltas; i++)
    {
        demodeltaitem *item = &delta->itemdeltas[i];
        writeint(item->type, &cp);
        writeint(item->id, &cp);

        // TODO: 1 <= type <= 20 is 0.6 protocol spesific
        if (!(item->type >= 1 && item->type <= 20))
            writeint(item->size, &cp);

        for (int i = 0; i < item->size; i++)
            writeint(item->data[i], &cp);
    }

    int size = cp - decompressed;

    int datasize = compresshuff(decompressed, size, compressed, BUFF_SIZE);

    writedemochunkheader(fp, DEMODELTA, datasize);
    fwrite(compressed, 1, datasize, fp);

    return 1;
}

int writedemo(FILE *fp, demo *demo)
{
    if (writedemoheader(fp, &demo->header) <= 0)
        return -1;

    if (writedemotimeline(fp, &demo->timeline) <= 0)
        return -2;

    if (writedemomap(fp, &demo->map, demo->header.mapsize, demo->header.version) <= 0)
        return -3;

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        demochunk *chunk = &demo->data.chunks[i];
        switch (chunk->type)
        {
        case DEMOTICK:
            writedemotick(fp, chunk->data.tick, demo->header.version);
            break;
        case DEMOSNAP:
            writedemosnap(fp, chunk->data.snap);
            break;
        case DEMOMESSAGE:
            writedemomessage(fp, chunk->data.message);
            break;
        case DEMODELTA:
            writedemodelta(fp, chunk->data.delta);
            break;
        default:
            printf("UNKNOWN CHUNK\n");
            break;
        }
    }

    return 1;
}

/* printers */

void printdemoheader(demoheader *header)
{
    if (header->version == 6)
        printf("version:    v6ddnet\n");
    else
        printf("version:    v%d\n", header->version);

    printf("netversion: %s\n", header->netversion);
    printf("mapname:    %s\n", header->mapname);
    printf("mapsize:    %d\n", header->mapsize);
    printf("mapcrc:     %d\n", header->mapcrc);
    printf("type:       %s\n", header->type);
    printf("length:     %d\n", header->length);
    printf("timestamp:  %s\n", header->timestamp);
}

void printdemotick(demotick *tick)
{
    printf("TICK={keyframe: %s, innline: %s, delta: %d}\n", tick->keyframe ? "true" : "false",
           tick->innline ? "true" : "false", tick->delta);
}

void printdemosnap(demosnap *snap)
{
    printf("SNAPSHOT={datasize: %d, numitems: %d,\n  offsets: [ ", snap->datasize, snap->numitems);
    for (int i = 0; i < snap->numitems; i++)
        printf("%d%s ", snap->offsets[i], (i != snap->numitems - 1) ? "," : "");

    printf("],\n  items: [\n");
    for (int i = 0; i < snap->numitems; i++)
    {
        printf("    (type: %d, id: %d) {", snap->items[i].type, snap->items[i].id);
        for (int y = 0; y < snap->items[i].numdata; y++)
            printf("%x%s", snap->items[i].data[y], (y != snap->items[i].numdata - 1) ? "," : "");
        printf("}\n");
    }
    printf("]}\n");
}

void printdemomessage(demomessage *message)
{
    printf("MESSAGE={datasize: %d}\n", message->datasize);
}

void printdemodelta(demodelta *delta)
{
    printf("DELTA={numremoveditems: %d, numitemdeltas: %d\n  removeditems: [ ", delta->numremoveditems,
           delta->numitemdeltas);

    for (int i = 0; i < delta->numremoveditems; i++)
    {
        short type = (delta->removeditemkeys[i] >> 16) & 0xffff;
        short id = (delta->removeditemkeys[i] & 0xffff);
        printf("(type: %d, id: %d)%s ", type, id, (i != delta->numremoveditems - 1) ? "," : "");
    }

    printf("],\n  itemdeltas: [\n");
    for (int i = 0; i < delta->numitemdeltas; i++)
    {
        printf("    (type: %d, id: %d) {", delta->itemdeltas[i].type, delta->itemdeltas[i].id);
        for (int y = 0; y < delta->itemdeltas[i].size; y++)
            printf("%x%s", delta->itemdeltas[i].data[y], (y != delta->itemdeltas[i].size - 1) ? "," : "");
        printf("}\n");
    }
    printf("]}\n");
}

void printdemo(demo *demo, char printchunks)
{
    printdemoheader(&demo->header);
    printf("\n");

    if (printchunks)
        printf("CHUNKS:\n");

    int typecount[4] = {0, 0, 0, 0};
    for (int i = 0; i < demo->data.numchunks; i++)
    {
        demochunk *chunk = &demo->data.chunks[i];
        switch (chunk->type)
        {
        case DEMOTICK:
            if (printchunks)
                printdemotick(chunk->data.tick);
            typecount[(int)DEMOTICK]++;
            break;
        case DEMOSNAP:
            if (printchunks)
                printdemosnap(chunk->data.snap);
            typecount[(int)DEMOSNAP]++;
            break;
        case DEMOMESSAGE:
            if (printchunks)
                printdemomessage(chunk->data.message);
            typecount[(int)DEMOMESSAGE]++;
            break;
        case DEMODELTA:
            if (printchunks)
                printdemodelta(chunk->data.delta);
            typecount[(int)DEMODELTA]++;
            break;
        default:
            printf("UNKNOWN CHUNK\n");
            break;
        }
    }
    if (printchunks)
        printf("\n");

    printf("chunk count: %d\n", demo->data.numchunks);
    printf("chunk counts: [\n");
    printf("  ticks: %d\n", typecount[0]);
    printf("  snaps: %d\n", typecount[1]);
    printf("  messages: %d\n", typecount[2]);
    printf("  deltas: %d\n", typecount[3]);
    printf("]\n");
}
