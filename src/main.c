#include "../inc/args.h"
#include "../inc/commands.h"
#include "../inc/demo.h"
#include "../inc/huffman.h"

#include <stdio.h>
#include <stdlib.h>

int mapname(char *mappath, char *out)
{
    int start = 0;
    int end;
    int i = 0;
    while (1)
    {
        if (mappath[i] == '/')
            start = i + 1;
        if (mappath[i] == '.' || mappath[i] == '\0')
        {
            end = i;
            break;
        }
        i++;
    }

    if ((end - start) > 31)
        return 0;

    i = 0;
    while (start != end)
    {
        out[i] = mappath[start];
        start++;
        i++;
    }
    out[i] = '\0';
    return 1;
}

FILE *demofile = NULL, *outfile = NULL, *mapfile = NULL, *extmapfile = NULL;
demo DEMO;

void exitperror(char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

void setdemo(arg *argument)
{
    demofile = fopen(argument->opts[0], "r");
    if (!demofile)
    {
        fprintf(stderr, "Error: failed to open demofile '%s', ", argument->opts[0]);
        exitperror("reason");
    }
    if (readdemo(demofile, &DEMO) < 0)
    {
        fprintf(stderr, "Error: failed to parse demo\n");
        exit(EXIT_FAILURE);
    }
    fclose(demofile);
}

void setmap(arg *argument)
{
    mapfile = fopen(argument->opts[0], "r");
    if (!mapfile)
        exitperror("Error: failed to open mapfile, reason");

    char mapnamebuff[32];
    if (mapname(argument->opts[0], mapnamebuff))
    {
        if (changemap(mapfile, mapnamebuff, &DEMO) < 1)
        {
            fprintf(stderr, "Error: failed to set map '%s'\n", argument->opts[0]);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Error: map name is too long (> 32)\n");
        exit(EXIT_FAILURE);
    }
    fclose(mapfile);
}

void runrename(arg *renamearg)
{
    int intid;
    int ret;
    if (sscanf(renamearg->opts[0], "%d", &intid) == 1)
        ret = setnamebyid(intid, renamearg->opts[1], &DEMO);
    else
        ret = setnamebyname(renamearg->opts[0], renamearg->opts[1], &DEMO);

    if (ret < 0)
    {
        printf("Error: failed to rename player '%s', reason: name too long\n", renamearg->opts[0]);
        exit(EXIT_FAILURE);
    }
}

void runsetskin(arg *skinarg)
{
    int intid;
    int ret;
    if (sscanf(skinarg->opts[0], "%d", &intid) == 1)
        ret = setskinbyid(intid, skinarg->opts[1], &DEMO);
    else
        ret = setskinbyname(skinarg->opts[0], skinarg->opts[1], &DEMO);

    if (ret < 0)
    {
        printf("Error: failed to set skin of player '%s', reason: skin name too long\n", skinarg->opts[0]);
        exit(EXIT_FAILURE);
    }
}

void runextractmap(arg *extarg)
{
    extmapfile = fopen(extarg->opts[0], "w");
    if (!extmapfile)
        exitperror("Error: failed to write to extract map file, reason");

    if (exportmap(extmapfile, &DEMO) < 0)
    {
        fprintf(stderr, "Error: failed to write demo map to extract map file\n");
        exit(EXIT_FAILURE);
    }
    fclose(extmapfile);
}

void runoutput(arg *outarg)
{
    outfile = fopen(outarg->opts[0], "w");
    if (!outfile)
        exitperror("Error: failed to write to outputfile, reason");

    if (writedemo(outfile, &DEMO) < 0)
    {
        fprintf(stderr, "Error: failed to write demo file to outputfile\n");
        exit(EXIT_FAILURE);
    }
    fclose(outfile);
}

int main(int argc, char *argv[])
{
    inithuff(NULL);

    setinfo("d(emo)edit, a tool for manipulating teeworlds demos");
    setusage("dedit <demo> [OPTIONS]");

    addarg("<demo>", "Sets input demo", setdemo);

    addopt("-r", "--rename", 2, "<id/name> <name>", "Renames player with id/name to name", runrename);
    addopt("-s", "--skin", 2, "<id/name> <skin>", "Set skin of player with id/name to skin", runsetskin);
    addopt("-m", "--map", 1, "<map>", "Changes the map of demo to map", setmap);
    addopt("-e", "--extract-map", 1, "<file>", "Saves the map of demo to file", NULL);
    addopt("-o", "--output", 1, "<file>", "Saves the output demo to file", NULL);
    addopt("-i", "--info", 0, NULL, "Prints info of demo", NULL);
    addopt("-I", "--extended-info", 0, NULL, "Prints extended info of demo", NULL);
    addopt("-h", "--help", 0, NULL, "Prints this help info", NULL);

    if (!parseargs(argc, argv))
    {
        paerror("Error:");
        printf("For more help run 'dedit --help'\n");
    }

    if (argc == 1 || getopt("-h", 0))
    {
        printhelp();
        exit(EXIT_SUCCESS);
    }

    arg *demoarg = getarg(0);
    if (demoarg == NULL)
    {
        printf("Error: Missing required argument <demo>\n");
        exit(EXIT_FAILURE);
    }

    demoarg->runarg(demoarg);

    arg *outarg = getopt("-o", 0);
    arg *maparg = getopt("-m", 0);
    arg *exmarg = getopt("-e", 0);
    arg *infarg = getopt("-i", 0);
    arg *Infarg = getopt("-I", 0);

    if (exmarg)
        runextractmap(exmarg);

    if (maparg)
        setmap(maparg);

    if (Infarg)
        printdemo(&DEMO, 1);
    else if (infarg)
        printdemo(&DEMO, 0);

    int i = 0;
    arg *opt;
    while (1)
    {
        opt = getopt("-s", i);
        if (opt == NULL)
            break;
        opt->runarg(opt);
        i++;
    }

    i = 0;
    while (1)
    {
        opt = getopt("-r", i);
        if (opt == NULL)
            break;
        opt->runarg(opt);
        i++;
    }

    if (outarg)
        runoutput(outarg);

    freeargs();

    exit(EXIT_SUCCESS);
}
