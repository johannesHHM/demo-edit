#ifndef COMMANDS_H
#define COMMANDS_H

#include "demo.h"

typedef struct
{
    char cmdtype;
    char settype;
    int id;
    char from[24];
    char to[24];
} cmdinput;

int setnamebyid(int id, char *newname, demo *demo);
int setnamebyname(char *oldname, char *newname, demo *demo);

int setskinbyid(int id, char *skin, demo *demo);
int setskinbyname(char *name, char *skin, demo *demo);

/* Changes demos map to given map */
/* Changes demo version to 5 if it is 6 */
/* Will fail if mapname >= 31, or map is not a teeworlds datafile */
int changemap(FILE *map, char *mapname, demo *demo);

void runcommand(cmdinput *cmd, demo *demo);

#endif // COMMANDS_H
