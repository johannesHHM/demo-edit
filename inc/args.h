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

#ifndef ARGS_H
#define ARGS_H

#define ARGTYPELEN 3
typedef enum
{
    ARGUMENT = 0,
    COMMAND = 1,
    OPTION = 2
} argtype;

typedef struct arg
{
    argtype type;
    char *flag;
    int numopt;
    char **opts;
    void (*runarg)(struct arg *);
} arg;

extern int argerr; // TODO should this be external?

extern int ARGC;
extern arg *ARGS;

void setinfo(char *info);
void setusage(char *usage);

int addarg(char *value, char *desc, void (*runarg)(struct arg *));
int addopt(char *flag, char *full, int numopt, char *value, char *desc, void (*runarg)(struct arg *));

int parseargs(int argc, char *args[]);
void freeargs(void);
void runargs(void);

arg *getarg(int pos);
arg *getopt(char *flag, int pos);

void printhelp(void);

void paerror(char *str);

#endif // ARGS_H
