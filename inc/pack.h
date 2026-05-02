/* This file is part of dedit - a tool for editing Teeworlds/DDNet demo files
   Copyright (C) 2024 JHHM

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
