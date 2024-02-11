#ifndef ARGS_H
#define ARGS_H

#include "../inc/commands.h"

#define MAX_COMMANDS 64

typedef struct 
{
    char *demopath;
    char *mappath;
    char *outpath;
    char *extractmap;
    char print;

    int numcmds;
    cmdinput commands[MAX_COMMANDS];
} input;

void parseargs(input *in, int argc, char *argv[]);

#endif // ARGS_H
