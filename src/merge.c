/* This file is part of dedit - a tool for editing Teeworlds/DDNet demo files
   Copyright (C) 2026 JHHM

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

#include "merge.h"
#include "demo.h"
#include "pack.h"
#include "snap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TICK_SPEED 50
#define KEYFRAME_INTERVAL (TICK_SPEED * 5)
#define MERGE_MAX_STALE_TICKS TICK_SPEED
#define MAX_TIMELINE_MARKERS 64
#define OFFSET_UUID_TYPE 0x4000
#define ITEMTYPE_EX 0

typedef enum
{
    ITEMSKIP = 0,      // never merged
    ITEMWORLD = 1,     // merged if the primary demo does not have it
    ITEMEVENT = 2,     // merged if the primary demo does not have it, and it was recorded at the tick
    ITEMPLAYERINFO = 3 // merged if the primary demo does not have it, and is not marked as local
} itemkind;

static const itemkind itemkinds06[] = {
    ITEMSKIP,       // id  0, ex
    ITEMSKIP,       // id  1, player input
    ITEMWORLD,      // id  2, projectile
    ITEMWORLD,      // id  3, laser
    ITEMWORLD,      // id  4, pickup
    ITEMWORLD,      // id  5, flag
    ITEMSKIP,       // id  6, game info
    ITEMSKIP,       // id  7, game data
    ITEMSKIP,       // id  8, character core
    ITEMWORLD,      // id  9, character
    ITEMPLAYERINFO, // id 10, player info
    ITEMWORLD,      // id 11, client info
    ITEMSKIP,       // id 12, spectator info
    ITEMEVENT,      // id 13, common
    ITEMEVENT,      // id 14, explosion
    ITEMEVENT,      // id 15, spawn
    ITEMEVENT,      // id 16, hammer hit
    ITEMEVENT,      // id 17, death
    ITEMEVENT,      // id 18, sound global
    ITEMEVENT,      // id 19, sound world
    ITEMEVENT       // id 20, damage indicator
};

typedef struct
{
    unsigned int uuid[4];
    itemkind kind;
} extype;

static const extype extypes[] = {
    {{0x76ce455b, 0xf9eb3a48, 0xadd7e04b, 0x941d045c}, ITEMWORLD}, // character@netobj
    {{0x22ca938d, 0x13803e2b, 0x9e7bd255, 0x8ea6be11}, ITEMWORLD}, // player@netobj
    {{0x0e6db85c, 0x2b61386f, 0xbbf2d0d0, 0x471b9272}, ITEMWORLD}, // projectile@netobj
    {{0x6550fbce, 0xf3173b31, 0x8ffed2b3, 0x7f3ab40e}, ITEMWORLD}, // ddnet-projectile@netobj
    {{0x29de68a2, 0x692831b8, 0x8360a230, 0x7e0d844f}, ITEMWORLD}, // laser@netobj
    {{0xea5e4a51, 0x58fb3684, 0x96e4e0d2, 0x67f4ca65}, ITEMWORLD}, // pickup@netobj
    {{0x2de9aec3, 0x32e43986, 0x8f7ee745, 0x9da7f535}, ITEMWORLD}, // entity-ex@netobj
    {{0x1fd35746, 0x6263358c, 0xb4d66ef6, 0x0e0efaaa}, ITEMEVENT}, // birthday@netevent
    {{0x68bf8939, 0xef553878, 0x90821352, 0x7eb0a597}, ITEMEVENT}, // finish@netevent
    {{0x54ecad2e, 0xbfad3be5, 0x8903621b, 0xa052458e}, ITEMEVENT}, // map-sound-world@netevent
};

/* demowriter struct, builds demo chunks like the demo recorder */
typedef struct
{
    demodata data;
    int chunkcap;
    unsigned char version;
    int firsttick;
    int lasttickmarker;
    int lastkeyframe;
    demosnap lastsnap;
} demowriter;

/* Returns how items with type in snap are merged */
/* Extended item types are looked up by their uuid in snap */
static itemkind getitemkind(const demosnap *snap, int type)
{
    if (type < (int)(sizeof(itemkinds06) / sizeof(itemkinds06[0])))
        return itemkinds06[type];

    if (type < OFFSET_UUID_TYPE)
        return ITEMSKIP;

    int index = snapfinditem(snap, ITEMTYPE_EX, type);
    if (index == -1 || snap->items[index].numdata != 4)
        return ITEMSKIP;

    for (size_t i = 0; i < sizeof(extypes) / sizeof(extypes[0]); i++)
    {
        if (memcmp(snap->items[index].data, extypes[i].uuid, sizeof(extypes[i].uuid)) == 0)
            return extypes[i].kind;
    }
    return ITEMSKIP;
}

/* Returns 1 if the extended item type has the same uuid in both snapshots, else 0 */
static int sameextype(const demosnap *a, const demosnap *b, int type)
{
    int aindex = snapfinditem(a, ITEMTYPE_EX, type);
    int bindex = snapfinditem(b, ITEMTYPE_EX, type);

    if (aindex == -1 || bindex == -1)
        return 0;

    const demosnapitem *aitem = &a->items[aindex];
    const demosnapitem *bitem = &b->items[bindex];

    if (aitem->numdata != bitem->numdata)
        return 0;

    return memcmp(aitem->data, bitem->data, aitem->numdata * sizeof(int)) == 0;
}

/* Merges the snapshots of primary and secondary at tick into out */
static int mergesnaps(const demostate *primary, const demostate *secondary, int tick, demosnap *out,
                      char *fullwarned)
{
    const demosnap *psnap = &primary->snap;
    int size = 2 * sizeof(int);

    snapinit(out);

    for (int i = 0; primary->hassnap && i < psnap->numitems; i++)
    {
        const demosnapitem *item = &psnap->items[i];

        if (primary->tick != tick && getitemkind(psnap, item->type) == ITEMEVENT)
            continue;

        if (snapadditem(out, item->type, item->id, item->numdata, item->data) < 0)
            return -1;
        size += (2 + item->numdata) * sizeof(int);
    }

    for (int i = 0; secondary && secondary->hassnap && i < secondary->snap.numitems; i++)
    {
        const demosnap *ssnap = &secondary->snap;
        const demosnapitem *item = &ssnap->items[i];
        itemkind kind = getitemkind(ssnap, item->type);
        int itemsize = (2 + item->numdata) * sizeof(int);

        if (kind == ITEMSKIP)
            continue;

        if (kind == ITEMEVENT && secondary->tick != tick)
            continue;

        if (item->type >= OFFSET_UUID_TYPE && !sameextype(psnap, ssnap, item->type))
            continue;

        if (snapfinditem(out, item->type, item->id) != -1)
            continue;

        if (out->numitems >= SNAP_MAX_ITEMS || size + itemsize > SNAP_MAX_SIZE)
        {
            if (!*fullwarned)
                fprintf(stderr, "Warning: merged snapshot is full, some items are dropped\n");
            *fullwarned = 1;
            continue;
        }

        if (snapadditem(out, item->type, item->id, item->numdata, item->data) < 0)
            return -1;
        size += itemsize;

        // there can only be one local player
        if (kind == ITEMPLAYERINFO && item->numdata > 0)
            out->items[out->numitems - 1].data[0] = 0;
    }

    return snapupdateoffsets(out);
}

/* writer */

static void writerinit(demowriter *w, unsigned char version)
{
    w->data.numchunks = 0;
    w->data.chunks = NULL;
    w->chunkcap = 0;
    w->version = version;
    w->firsttick = -1;
    w->lasttickmarker = -1;
    w->lastkeyframe = -1;
    snapinit(&w->lastsnap);
}

static void writerfree(demowriter *w)
{
    freedemodata(&w->data);
    snapfree(&w->lastsnap);
}

static int writechunk(demowriter *w, demochunktype type, chunkdata data)
{
    if (w->data.numchunks >= w->chunkcap)
    {
        int chunkcap = w->chunkcap ? w->chunkcap * 2 : 1024;
        demochunk *chunks = (demochunk *)realloc(w->data.chunks, chunkcap * sizeof(demochunk));
        if (chunks == NULL)
            return -1;
        w->data.chunks = chunks;
        w->chunkcap = chunkcap;
    }

    w->data.chunks[w->data.numchunks].type = type;
    w->data.chunks[w->data.numchunks].data = data;
    w->data.numchunks++;

    return 1;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/master/src/engine/shared/demo.cpp (CDemoRecorder::WriteTickMarker)
*/
static int writetickmarker(demowriter *w, int tick, char keyframe)
{
    int maxinline = (w->version >= 5) ? 0x1f : 0x3f;
    demotick *t = (demotick *)malloc(sizeof(demotick));
    chunkdata data;

    if (t == NULL)
        return -1;

    t->keyframe = keyframe;
    if (keyframe || w->lasttickmarker == -1 || tick <= w->lasttickmarker || tick - w->lasttickmarker > maxinline)
    {
        t->innline = 0;
        t->delta = tick;
    }
    else
    {
        t->innline = 1;
        t->delta = tick - w->lasttickmarker;
    }

    data.tick = t;
    if (writechunk(w, DEMOTICK, data) < 0)
    {
        free(t);
        return -1;
    }

    w->lasttickmarker = tick;
    if (w->firsttick == -1)
        w->firsttick = tick;

    return 1;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/master/src/engine/shared/demo.cpp (CDemoRecorder::RecordSnapshot)
*/
static int writesnapshot(demowriter *w, int tick, const demosnap *snap)
{
    demodelta delta = {0, 0, NULL, NULL};
    char keyframe = (w->lastkeyframe == -1 || tick - w->lastkeyframe > KEYFRAME_INTERVAL);
    int ret = 0;
    chunkdata data;

    if (!keyframe)
    {
        ret = snapcreatedelta(&w->lastsnap, snap, &delta);
        // an item changed size, only a full snapshot can store that
        if (ret == -2)
            keyframe = 1;
        else if (ret < 0)
            return -1;
    }

    if (writetickmarker(w, tick, keyframe) < 0)
    {
        deltafree(&delta);
        return -1;
    }

    if (keyframe)
    {
        demosnap *full = (demosnap *)malloc(sizeof(demosnap));
        if (full == NULL)
            return -1;

        if (snapcopy(full, snap) < 0)
        {
            free(full);
            return -1;
        }

        data.snap = full;
        if (writechunk(w, DEMOSNAP, data) < 0)
        {
            snapfree(full);
            free(full);
            return -1;
        }
        w->lastkeyframe = tick;
    }
    else if (ret > 0)
    {
        demodelta *d = (demodelta *)malloc(sizeof(demodelta));
        if (d == NULL)
        {
            deltafree(&delta);
            return -1;
        }

        *d = delta;
        data.delta = d;
        if (writechunk(w, DEMODELTA, data) < 0)
        {
            deltafree(d);
            free(d);
            return -1;
        }
    }
    else
    {
        // snapshot is unchanged, only the tick marker is written
        deltafree(&delta);
        return 1;
    }

    snapfree(&w->lastsnap);
    return snapcopy(&w->lastsnap, snap);
}

static int writemessage(demowriter *w, const demomessage *message)
{
    demomessage *copy = (demomessage *)malloc(sizeof(demomessage));
    chunkdata data;

    if (copy == NULL)
        return -1;

    copy->datasize = message->datasize;
    copy->data = (char *)malloc(message->datasize > 0 ? message->datasize : 1);
    if (copy->data == NULL)
    {
        free(copy);
        return -1;
    }
    memcpy(copy->data, message->data, message->datasize);

    data.message = copy;
    if (writechunk(w, DEMOMESSAGE, data) < 0)
    {
        free(copy->data);
        free(copy);
        return -1;
    }

    return 1;
}

/* Writes the chunks of the current tick of state, with snap as the snapshot of the tick */
static int writetick(demowriter *w, const demostate *state, int tick, const demosnap *snap)
{
    demodata *data = &state->demo->data;

    for (int i = state->firstchunk; i < state->chunkidx; i++)
    {
        demochunk *chunk = &data->chunks[i];
        if (chunk->type == DEMOTICK)
        {
            if (writesnapshot(w, tick, snap) < 0)
                return -1;
        }
        else if (chunk->type == DEMOMESSAGE)
        {
            if (writemessage(w, chunk->data.message) < 0)
                return -1;
        }
    }

    return 1;
}

static void mergetimelines(demotimeline *base, const demotimeline *other)
{
    const demotimeline *timelines[2] = {base, other};
    int markers[2 * MAX_TIMELINE_MARKERS];
    int nummarkers = 0;

    for (int t = 0; t < 2; t++)
    {
        const unsigned char *data = (const unsigned char *)timelines[t]->data;
        int num = frombigendian(data);

        if (num < 0)
            num = 0;
        if (num > MAX_TIMELINE_MARKERS)
            num = MAX_TIMELINE_MARKERS;

        for (int i = 0; i < num; i++)
        {
            int marker = frombigendian(data + 4 + 4 * i);
            int pos = nummarkers;

            // insertion sort, skipping duplicates
            while (pos > 0 && markers[pos - 1] > marker)
                pos--;
            if (pos > 0 && markers[pos - 1] == marker)
                continue;

            memmove(&markers[pos + 1], &markers[pos], (nummarkers - pos) * sizeof(int));
            markers[pos] = marker;
            nummarkers++;
        }
    }

    if (nummarkers > MAX_TIMELINE_MARKERS)
        nummarkers = MAX_TIMELINE_MARKERS;

    unsigned char *data = (unsigned char *)base->data;
    memset(data, 0, DEMO_TIMELINE_LENGTH);
    tobigendian(nummarkers, data);
    for (int i = 0; i < nummarkers; i++)
        tobigendian(markers[i], data + 4 + 4 * i);
}

int mergedemo(demo *base, demo *other)
{
    demostate a, b;
    demowriter w;
    char fullwarned = 0;
    int ret = 1;

    if (strncmp(base->header.mapname, other->header.mapname, sizeof(base->header.mapname)) != 0 ||
        base->header.mapcrc != other->header.mapcrc)
        fprintf(stderr, "Warning: demos have different maps\n");

    if (strncmp(base->header.netversion, other->header.netversion, sizeof(base->header.netversion)) != 0)
        fprintf(stderr, "Warning: demos have different network versions\n");

    demostateinit(&a, base);
    demostateinit(&b, other);

    if (a.firsttick != -1 && b.firsttick != -1 && (a.lasttick < b.firsttick || b.lasttick < a.firsttick))
        fprintf(stderr, "Warning: demos do not overlap in time, are they recorded on the same server?\n");

    writerinit(&w, base->header.version);

    while (1)
    {
        int atick = demostatepeektick(&a);
        int btick = demostatepeektick(&b);
        char anext = 0, bnext = 0, primarynext;
        demostate *primary, *secondary;
        demosnap merged;
        int tick;

        if (atick == -1 && btick == -1)
            break;

        if (atick == -1)
            tick = btick;
        else if (btick == -1)
            tick = atick;
        else
            tick = (atick < btick) ? atick : btick;

        // advance demos that have the tick
        if (atick == tick)
        {
            if (demostatenext(&a) < 0)
            {
                ret = -2;
                break;
            }
            anext = 1;
        }
        if (btick == tick)
        {
            if (demostatenext(&b) < 0)
            {
                ret = -2;
                break;
            }
            bnext = 1;
        }

        if (a.tick != -1 && tick <= a.lasttick)
        {
            primary = &a;
            secondary = &b;
            primarynext = anext;
        }
        else
        {
            primary = &b;
            secondary = &a;
            primarynext = bnext;
        }

        // only use secondary while it is recording and has recent data

        if (secondary->tick == -1 || tick > secondary->lasttick || tick - secondary->tick > MERGE_MAX_STALE_TICKS)
            secondary = NULL;

        if (mergesnaps(primary, secondary, tick, &merged, &fullwarned) < 0)
        {
            snapfree(&merged);
            ret = -3;
            break;
        }

        if (primarynext)
            ret = writetick(&w, primary, tick, &merged);
        else
            ret = writesnapshot(&w, tick, &merged);

        snapfree(&merged);
        if (ret < 0)
        {
            ret = -3;
            break;
        }
    }

    demostatefree(&a);
    demostatefree(&b);

    if (ret < 0)
    {
        writerfree(&w);
        return ret;
    }

    if (b.firsttick != -1 && (a.firsttick == -1 || b.firsttick < a.firsttick))
        memcpy(base->header.timestamp, other->header.timestamp, sizeof(base->header.timestamp));

    if (w.firsttick != -1)
        base->header.length = (w.lasttickmarker - w.firsttick) / TICK_SPEED;

    mergetimelines(&base->timeline, &other->timeline);

    freedemodata(&base->data);
    base->data = w.data;
    snapfree(&w.lastsnap);

    return 1;
}
