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
void runargs();

arg *getarg(int pos);
arg *getopt(char *flag, int pos);

void printhelp();

void paerror(char *str);

#endif // ARGS_H
