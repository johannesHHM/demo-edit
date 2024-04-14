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
