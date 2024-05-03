#include "../inc/args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct parg
{
    argtype type;
    char *flag;
    char *full;
    int numopt;
    char *desc[2];
    void (*runarg)(struct arg *);
} parg;

int argerr = 0;
char *errflag = NULL;

char *inf = NULL;
char *usg = NULL;

int PARGC = 0;
int pargscap = 2;
parg *PARGS = NULL;

int ARGC = 0;
int argscap = 2;
arg *ARGS;

void setinfo(char *info)
{
    inf = info;
}

void setusage(char *usage)
{
    usg = usage;
}

int addparg(argtype type, char *flag, char *full, int numopt, char *desc1, char *desc2, void (*runarg)(struct arg *))
{
    if (!PARGS)
        PARGS = (parg *)malloc(pargscap * sizeof(parg));
    else
    {
        pargscap *= 2;
        PARGS = (parg *)realloc(PARGS, pargscap * sizeof(parg));
    }
    PARGS[PARGC].type = type;
    PARGS[PARGC].flag = flag;
    PARGS[PARGC].full = full;
    PARGS[PARGC].numopt = numopt;
    PARGS[PARGC].desc[0] = desc1;
    PARGS[PARGC].desc[1] = desc2;
    PARGS[PARGC].runarg = runarg;
    PARGC++;

    return 1;
}

int addarg(char *value, char *desc, void (*runarg)(struct arg *))
{
    if (!value || !desc)
        return 0;
    addparg(ARGUMENT, NULL, NULL, 0, value, desc, runarg);
    return 1;
}

int addopt(char *flag, char *full, int numopt, char *value, char *desc, void (*runarg)(struct arg *))
{
    if (!flag || !desc)
        return 0;
    addparg(OPTION, flag, full, numopt, value, desc, runarg);
    return 1;
}

int checkarg(char *str, parg *par)
{
    if (par->flag && strcmp(str, par->flag) == 0)
        return 1;

    if (par->full && strcmp(str, par->full) == 0)
        return 1;

    return 0;
}

int parseargs(int argc, char *args[])
{
    ARGS = (arg *)malloc(argscap * sizeof(arg));

    char inctype[] = {0, 0, 0};
    for (int i = 0; i < PARGC; i++)
        inctype[PARGS[i].type]++;

    int argumentc = 0;

    for (int i = 1; i < argc; i++)
    {
        char *a = args[i];
        char isopt = 0;
        for (int y = 0; y < PARGC; y++)
        {
            parg *pa = &PARGS[y];
            if (checkarg(a, pa))
            {
                if (i + pa->numopt >= argc)
                {
                    argerr = 1;
                    errflag = pa->flag;
                    return 0;
                }
                isopt = 1;
                if (argscap >= ARGC)
                {
                    argscap *= 2;
                    ARGS = (arg *)realloc(ARGS, argscap * sizeof(arg));
                }
                ARGS[ARGC].type = pa->type;
                ARGS[ARGC].flag = pa->flag;
                ARGS[ARGC].numopt = pa->numopt;
                ARGS[ARGC].runarg = pa->runarg;
                ARGS[ARGC].opts = (char **)malloc(pa->numopt * sizeof(char *));

                for (int j = 0; j < pa->numopt; j++)
                    ARGS[ARGC].opts[j] = args[i + j + 1];

                i += pa->numopt;
                ARGC++;
            }
        }
        if (isopt)
            continue;

        if (argumentc < inctype[ARGUMENT])
        {
            ARGS[ARGC].type = ARGUMENT;
            for (int y = 0, x = argumentc; y < PARGC; y++)
            {
                if (PARGS[y].type == ARGUMENT)
                {
                    if (x != 0)
                        x--;
                    else
                    {
                        ARGS[ARGC].numopt = 1;
                        ARGS[ARGC].flag = "NONE";
                        ARGS[ARGC].runarg = PARGS[y].runarg;
                        ARGS[ARGC].opts = (char **)malloc(sizeof(char *));
                        ARGS[ARGC].opts[0] = a;
                        argumentc++;
                        ARGC++;
                    }
                }
            }
        }
        else
        {
            if (a[0] != '-')
            {
                argerr = 2;
                errflag = NULL;
            }
            else
            {
                argerr = 3;
                errflag = args[i];
            }
            return 0;
        }
    }
    return 1;
}

void freeargs()
{
    for (int i = 0; i < ARGC; i++)
    {
        if (ARGS[i].opts)
            free(ARGS[i].opts);
    }
    if (ARGS)
        free(ARGS);
    if (PARGS)
        free(PARGS);
}

void runargs()
{
    for (int i = 0; i < ARGC; i++)
    {
        if (ARGS[i].runarg)
            ARGS[i].runarg(&ARGS[i]);
    }
}

arg *getarg(int pos)
{
    for (int i = 0; i < ARGC; i++)
        if (ARGS[i].type == ARGUMENT)
        {
            if (pos == 0)
                return &ARGS[i];
            pos--;
        }
    return NULL;
}

arg *getopt(char *flag, int pos)
{
    for (int i = 0; i < ARGC; i++)
        if (ARGS[i].type == OPTION && strcmp(flag, ARGS[i].flag) == 0)
        {
            if (pos == 0)
                return &ARGS[i];
            pos--;
        }
    return NULL;
}

void printhelp()
{
    char inctype[ARGTYPELEN] = {0, 0, 0};
    int maxflag[ARGTYPELEN] = {0, 0, 0};
    int maxfull[ARGTYPELEN] = {0, 0, 0};
    int maxdesc0[ARGTYPELEN] = {0, 0, 0};
    int maxdesc1[ARGTYPELEN] = {0, 0, 0};

    for (int i = 0; i < PARGC; i++)
    {
        parg *pa = &PARGS[i];
        int type = pa->type;

        inctype[type]++;
        if (pa->flag)
            maxflag[type] = ((size_t)maxflag[type] > strlen(pa->flag) ? maxflag[type] : (int)strlen(pa->flag));
        if (pa->full)
            maxfull[type] = ((size_t)maxfull[type] > strlen(pa->full) ? maxfull[type] : (int)strlen(pa->full));
        if (pa->desc[0])
            maxdesc0[type] = ((size_t)maxdesc0[type] > strlen(pa->desc[0]) ? maxdesc0[type] : (int)strlen(pa->desc[0]));
        if (pa->desc[1])
            maxdesc1[type] = ((size_t)maxdesc1[type] > strlen(pa->desc[1]) ? maxdesc1[type] : (int)strlen(pa->desc[1]));
    }
    if (inf)
        printf("%s\n\n", inf);

    if (usg)
        printf("\033[4mUsage:\033[0m %s\n", usg);

    if (inctype[ARGUMENT])
    {
        printf("\n\033[4mArgument%c:\033[0m\n", (inctype[ARGUMENT] < 2) ? '\0' : 's');
        for (int i = 0; i < PARGC; i++)
        {
            parg *pa = &PARGS[i];
            if (pa->type == ARGUMENT)
                printf("  %s %s\n", pa->desc[0], pa->desc[1]);
        }
    }

    if (inctype[OPTION])
    {
        printf("\n\033[4mOption%c:\033[0m\n", (inctype[OPTION] < 2) ? '\0' : 's');
        for (int i = 0; i < PARGC; i++)
        {
            parg *pa = &PARGS[i];
            if (pa->type == OPTION)
                printf("  %*s, %-*s %-*s  %s\n", maxflag[OPTION], pa->flag, maxfull[OPTION], pa->full, maxdesc0[OPTION],
                       (pa->desc[0]) ? pa->desc[0] : "", pa->desc[1]);
        }
    }
}

void paerror(char *str)
{
    char *errstr;
    switch (argerr)
    {
    case 0:
        errstr = "No error";
        break;
    case 1:
        errstr = "Missing input for option flag";
        break;
    case 2:
        errstr = "Too many positional arguments";
        break;
    case 3:
        errstr = "Unknown flag";
        break;
    default:
        errstr = "Unknown error";
        break;
    }
    if (errflag)
        fprintf(stderr, "%s %s '%s'\n", str, errstr, errflag);
    else
        fprintf(stderr, "%s %s\n", str, errstr);
}
