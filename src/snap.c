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

#include "snap.h"
#include "demo.h"

#include <stdlib.h>
#include <string.h>

void snapinit(demosnap *snap)
{
    snap->datasize = 0;
    snap->numitems = 0;
    snap->offsets = NULL;
    snap->items = NULL;
}

void snapfree(demosnap *snap)
{
    for (int i = 0; i < snap->numitems; i++)
        free(snap->items[i].data);
    free(snap->offsets);
    free(snap->items);
    snapinit(snap);
}

/* Sets item to a new item with type and id, copying numdata ints from data */
/* If data is NULL the item data is zeroed */
static int setitem(demosnapitem *item, int type, int id, int numdata, const int *data)
{
    item->type = type;
    item->id = id;
    item->numdata = numdata;
    item->data = (int *)calloc(numdata > 0 ? numdata : 1, sizeof(int));
    if (item->data == NULL)
        return -1;

    if (data != NULL && numdata > 0)
        memcpy(item->data, data, numdata * sizeof(int));

    return 1;
}

int snapcopy(demosnap *dst, const demosnap *src)
{
    snapinit(dst);
    if (src->numitems == 0)
        return 1;

    dst->items = (demosnapitem *)malloc(src->numitems * sizeof(demosnapitem));
    if (dst->items == NULL)
        return -1;

    for (int i = 0; i < src->numitems; i++)
    {
        const demosnapitem *item = &src->items[i];
        if (setitem(&dst->items[i], item->type, item->id, item->numdata, item->data) < 0)
        {
            snapfree(dst);
            return -1;
        }
        dst->numitems++;
    }

    if (snapupdateoffsets(dst) < 0)
    {
        snapfree(dst);
        return -1;
    }

    return 1;
}

int snapfinditem(const demosnap *snap, int type, int id)
{
    for (int i = 0; i < snap->numitems; i++)
    {
        if (snap->items[i].type == type && snap->items[i].id == id)
            return i;
    }
    return -1;
}

int snapadditem(demosnap *snap, int type, int id, int numdata, const int *data)
{
    demosnapitem *items = (demosnapitem *)realloc(snap->items, (snap->numitems + 1) * sizeof(demosnapitem));
    if (items == NULL)
        return -1;
    snap->items = items;

    if (setitem(&snap->items[snap->numitems], type, id, numdata, data) < 0)
        return -1;
    snap->numitems++;

    return 1;
}

int snapupdateoffsets(demosnap *snap)
{
    free(snap->offsets);
    snap->offsets = NULL;
    snap->datasize = 0;

    if (snap->numitems == 0)
        return 1;

    snap->offsets = (int *)malloc(snap->numitems * sizeof(int));
    if (snap->offsets == NULL)
        return -1;

    for (int i = 0; i < snap->numitems; i++)
    {
        snap->offsets[i] = snap->datasize;
        snap->datasize += (1 + snap->items[i].numdata) * sizeof(int);
    }

    return 1;
}

int snapsize(const demosnap *snap)
{
    return (2 + snap->numitems) * sizeof(int) + snap->datasize;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/master/src/engine/shared/snapshot.cpp (CSnapshotDelta::UnpackDelta)
*/
int snapapplydelta(const demosnap *from, const demodelta *delta, demosnap *to)
{
    int maxitems = from->numitems + delta->numitemdeltas;

    snapinit(to);
    to->items = (demosnapitem *)malloc((maxitems > 0 ? maxitems : 1) * sizeof(demosnapitem));
    if (to->items == NULL)
        return -1;

    // copy all items that are not removed
    for (int i = 0; i < from->numitems; i++)
    {
        const demosnapitem *item = &from->items[i];
        int key = SNAP_KEY(item->type, item->id);
        char keep = 1;

        for (int y = 0; y < delta->numremoveditems; y++)
        {
            if (delta->removeditemkeys[y] == key)
            {
                keep = 0;
                break;
            }
        }

        if (!keep)
            continue;

        if (setitem(&to->items[to->numitems], item->type, item->id, item->numdata, item->data) < 0)
        {
            snapfree(to);
            return -1;
        }
        to->numitems++;
    }

    // apply updated items
    for (int i = 0; i < delta->numitemdeltas; i++)
    {
        const demodeltaitem *ditem = &delta->itemdeltas[i];
        int index = snapfinditem(to, ditem->type, ditem->id);

        if (index == -1)
        {
            index = to->numitems;
            if (setitem(&to->items[index], ditem->type, ditem->id, ditem->size, NULL) < 0)
            {
                snapfree(to);
                return -1;
            }
            to->numitems++;
        }
        else if (to->items[index].numdata != ditem->size)
        {
            snapfree(to);
            return -2;
        }

        int *data = to->items[index].data;
        int fromindex = snapfinditem(from, ditem->type, ditem->id);

        if (fromindex != -1)
        {
            const demosnapitem *fromitem = &from->items[fromindex];
            if (fromitem->numdata != ditem->size)
            {
                snapfree(to);
                return -3;
            }

            // wrapping adition by casting to unsigned
            for (int y = 0; y < ditem->size; y++)
                data[y] = (int)((unsigned int)fromitem->data[y] + (unsigned int)ditem->data[y]);
        }
        else if (ditem->size > 0)
        {
            memcpy(data, ditem->data, ditem->size * sizeof(int));
        }
    }

    if (snapupdateoffsets(to) < 0)
    {
        snapfree(to);
        return -1;
    }

    return 1;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/master/src/engine/shared/snapshot.cpp (CSnapshotDelta::CreateDelta)
*/
int snapcreatedelta(const demosnap *from, const demosnap *to, demodelta *delta)
{
    delta->numremoveditems = 0;
    delta->numitemdeltas = 0;
    delta->removeditemkeys = (int *)malloc((from->numitems > 0 ? from->numitems : 1) * sizeof(int));
    delta->itemdeltas = (demodeltaitem *)malloc((to->numitems > 0 ? to->numitems : 1) * sizeof(demodeltaitem));

    if (delta->removeditemkeys == NULL || delta->itemdeltas == NULL)
    {
        deltafree(delta);
        return -1;
    }

    // removed items
    for (int i = 0; i < from->numitems; i++)
    {
        const demosnapitem *item = &from->items[i];
        if (snapfinditem(to, item->type, item->id) == -1)
            delta->removeditemkeys[delta->numremoveditems++] = SNAP_KEY(item->type, item->id);
    }

    // new and changed items
    for (int i = 0; i < to->numitems; i++)
    {
        const demosnapitem *item = &to->items[i];
        demodeltaitem *ditem = &delta->itemdeltas[delta->numitemdeltas];
        int fromindex = snapfinditem(from, item->type, item->id);

        if (fromindex != -1 && from->items[fromindex].numdata != item->numdata)
        {
            deltafree(delta);
            return -2;
        }

        ditem->type = item->type;
        ditem->id = item->id;
        ditem->size = item->numdata;
        ditem->data = (int *)malloc((item->numdata > 0 ? item->numdata : 1) * sizeof(int));
        if (ditem->data == NULL)
        {
            deltafree(delta);
            return -1;
        }

        if (fromindex != -1)
        {
            const demosnapitem *fromitem = &from->items[fromindex];
            int needed = 0;

            // wrapping subtraction by casting to unsigned
            for (int y = 0; y < item->numdata; y++)
            {
                ditem->data[y] = (int)((unsigned int)item->data[y] - (unsigned int)fromitem->data[y]);
                needed |= ditem->data[y];
            }

            if (!needed)
            {
                free(ditem->data);
                continue;
            }
        }
        else if (item->numdata > 0)
        {
            memcpy(ditem->data, item->data, item->numdata * sizeof(int));
        }
        delta->numitemdeltas++;
    }

    return (delta->numremoveditems > 0 || delta->numitemdeltas > 0) ? 1 : 0;
}

void deltafree(demodelta *delta)
{
    for (int i = 0; i < delta->numitemdeltas; i++)
        free(delta->itemdeltas[i].data);
    free(delta->itemdeltas);
    free(delta->removeditemkeys);

    delta->numremoveditems = 0;
    delta->numitemdeltas = 0;
    delta->removeditemkeys = NULL;
    delta->itemdeltas = NULL;
}

/* Returns the tick after tick marker t, given the previous tick */
static int nexttick(int tick, const demotick *t)
{
    if (t->innline)
        return tick + t->delta;
    return t->delta;
}

void demostateinit(demostate *state, demo *demo)
{
    int tick = -1;

    state->demo = demo;
    state->chunkidx = 0;
    state->firstchunk = 0;
    state->tick = -1;
    state->firsttick = -1;
    state->lasttick = -1;
    state->hassnap = 0;
    snapinit(&state->snap);

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        demochunk *chunk = &demo->data.chunks[i];
        if (chunk->type != DEMOTICK)
            continue;

        tick = nexttick(tick, chunk->data.tick);
        if (state->firsttick == -1)
            state->firsttick = tick;
        state->lasttick = tick;
    }
}

int demostatepeektick(const demostate *state)
{
    demodata *data = &state->demo->data;

    for (int i = state->chunkidx; i < data->numchunks; i++)
    {
        if (data->chunks[i].type == DEMOTICK)
            return nexttick(state->tick, data->chunks[i].data.tick);
    }
    return -1;
}

/*
  Function is derived from DDNet code (DDNet License).
  See NOTICES file for full license details.

  https://github.com/ddnet/ddnet/blob/master/src/engine/shared/demo.cpp (CDemoPlayer::DoTick)
*/
int demostatenext(demostate *state)
{
    demodata *data = &state->demo->data;
    char seentick = 0;
    demosnap snap;

    state->firstchunk = state->chunkidx;

    for (; state->chunkidx < data->numchunks; state->chunkidx++)
    {
        demochunk *chunk = &data->chunks[state->chunkidx];
        switch (chunk->type)
        {
        case DEMOTICK:
            if (seentick)
                return 1;
            seentick = 1;
            state->tick = nexttick(state->tick, chunk->data.tick);
            break;
        case DEMOSNAP:
            if (snapcopy(&snap, chunk->data.snap) < 0)
                return -1;
            snapfree(&state->snap);
            state->snap = snap;
            state->hassnap = 1;
            break;
        case DEMODELTA:
            if (!state->hassnap)
                return -2;
            if (snapapplydelta(&state->snap, chunk->data.delta, &snap) < 0)
                return -3;
            snapfree(&state->snap);
            state->snap = snap;
            break;
        default:
            break;
        }
    }

    return seentick;
}

void demostatefree(demostate *state)
{
    snapfree(&state->snap);
    state->hassnap = 0;
}
