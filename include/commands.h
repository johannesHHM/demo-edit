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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "demo.h"

int setnamebyid(int id, char *newname, demo *demo);
int setnamebyname(char *oldname, char *newname, demo *demo);

int setskinbyid(int id, char *skin, demo *demo);
int setskinbyname(char *name, char *skin, demo *demo);

/* Changes demos map to given map */
/* Changes demo version to 5 if it is 6 */
/* Will fail if mapname >= 31, or map is not a teeworlds datafile */
int changemap(FILE *map, char *mapname, demo *demo);

/* Writes the map in demo to out */
int exportmap(FILE *out, demo *demo);

#endif // COMMANDS_H
