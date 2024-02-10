#ifndef COMMANDS_H
#define COMMANDS_H

#include "demo.h"

int setnamebyid(int id, char *newname, demo *demo);
int setnamebyname(char *oldname, char *newname, demo *demo);

int setskinbyname(char *name, char *skin, demo *demo);

int changemap(FILE *map, char *mapname, demo *demo);

#endif // COMMANDS_H
