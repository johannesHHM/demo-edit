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

#ifndef HUFFMAN_H
#define HUFFMAN_H

/* Initialize huffman */
/* Uses the standard frequency table if NULL is provided */
void inithuff(const unsigned int *frequencies);

/* Compress contents of input buffer, result in output buffer */
/* Returns -1 on too small output buffer */
int compresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize);

/* Decompress contents of input buffer, result in output buffer */
/* Returns -1 on decompress error */
int decompresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize);

#endif // HUFFMAN_H
