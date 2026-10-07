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

#ifndef MERGE_H
#define MERGE_H

#include "demo.h"

/* Merge snapshot other into base */
/* Returns a negative number on error */
int mergedemo(demo *base, demo *other);

#endif // MERGE_H

