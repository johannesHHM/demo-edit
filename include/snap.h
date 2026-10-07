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

#ifndef SNAP_H
#define SNAP_H

#include "demo.h"

#define SNAP_MAX_ITEMS 1024
#define SNAP_MAX_SIZE (64 * 1024)
#define SNAP_KEY(type, id) (((type) << 16) | (id))

/* demostate struct, the full game state of a demo at a tick */
typedef struct
{
    demo *demo;
    int chunkidx;
    int firstchunk;
    int tick;
    int firsttick;
    int lasttick;
    char hassnap;
    demosnap snap;
} demostate;

/* Initializes empty snapshot */
void snapinit(demosnap *snap);

/* Frees the items of snap */
/* Sets snap to an empty snapshot */
void snapfree(demosnap *snap);

/* Copies src into dst, dst must not hold a snapshot */
/* Returns a positive number on success, negative on error */
int snapcopy(demosnap *dst, const demosnap *src);

/* Returns the index of the item with type and id in snap, -1 if not found */
int snapfinditem(const demosnap *snap, int type, int id);

/* Appends a new item to snap, copying numdata ints from data */
/* Offsets of snap must be updated with snapupdateoffsets afterwards */
/* Returns a positive number on success, negative on error */
int snapadditem(demosnap *snap, int type, int id, int numdata, const int *data);

/* Recalculates the offsets and datasize of snap from its items */
/* Returns a positive number on success, negative on error */
int snapupdateoffsets(demosnap *snap);

/* Returns the size of snap in bytes, as it is stored in a demo */
int snapsize(const demosnap *snap);

/* Applies delta to from, and stores the resulting snapshot in to */
/* to must not hold a snapshot */
/* Returns a positive number on success, negative on error */
int snapapplydelta(const demosnap *from, const demodelta *delta, demosnap *to);

/* Creates the delta from snapshot from to snapshot to */
/* Returns 0 if the snapshots are equal, positive number if delta is not empty, negative on error */
/* Returns -2 if an item changed size, which can not be stored in a delta */
/* Allocates memory needed in delta, unless an error occurred */
int snapcreatedelta(const demosnap *from, const demosnap *to, demodelta *delta);

/* Frees the item data of delta */
/* Sets delta to an empty delta */
void deltafree(demodelta *delta);

/* Initializes state at the start of demo, before the first tick */
void demostateinit(demostate *state, demo *demo);

/* Returns the next tick of state, -1 if there are no more ticks */
int demostatepeektick(const demostate *state);

/* Advances state to next tick, and calculates the full snapshot */
/* Returns a positive number on success, 0 on end of demo, and negative number on error */
int demostatenext(demostate *state);

/* Frees everything for state */
void demostatefree(demostate *state);

#endif // SNAP_H

