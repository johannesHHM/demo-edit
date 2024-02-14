#include "../inc/args.h"
#include "../inc/commands.h"
#include "../inc/demo.h"
#include "../inc/huffman.h"

#include <stdio.h>
#include <stdlib.h>

int testcompress(char *line)
{
    char inputbuff[4096];
    char outputbuff[4096];
    char resultbuff[4096];

    int inputlen = 0;
    int resultlen = 0;
    char *splitpos = line;

    while (*splitpos != '#')
        splitpos++;

    while (sscanf(line, "%2hhx", inputbuff + inputlen) == 1 && line < splitpos)
    {
        inputlen++;
        line += 3;
    }

    splitpos++;
    while (sscanf(splitpos, "%2hhx", resultbuff + resultlen) == 1)
    {
        resultlen++;
        splitpos += 3;
    }
    int i;

    int outputlen = compresshuff(inputbuff, inputlen, outputbuff, 4096);
    if (outputlen < 0)
    {
        printf("[FAILED TEST] Failed compress test! outputbuff too small\n");
        return -1;
    }

    if (outputlen != resultlen)
    {
        printf("[FAILED TEST] Failed compress test! different result output length\n");
        return -1;
    }

    for (i = 0; i < outputlen; i++)
    {
        if (resultbuff[i] != outputbuff[i])
        {
            printf("[FAILED TEST] Failed compress test!\n");
            return -1;
        }
    }
    return 1;
}

int testdecompress(char *line)
{
    char inputbuff[4096];
    char outputbuff[4096];
    char resultbuff[4096];

    int inputlen = 0;
    int resultlen = 0;
    char *splitpos = line;

    while (*splitpos != '#')
        splitpos++;

    while (sscanf(line, "%2hhx", inputbuff + inputlen) == 1 && line < splitpos)
    {
        inputlen++;
        line += 3;
    }

    splitpos++;
    while (sscanf(splitpos, "%2hhx", resultbuff + resultlen) == 1)
    {
        resultlen++;
        splitpos += 3;
    }
    int i;

    int outputlen = decompresshuff(resultbuff, inputlen, outputbuff, 4096);
    if (outputlen < 0)
    {
        printf("[FAILED TEST] Failed decompress test! Error when decompressing\n");
        return -1;
    }

    if (outputlen != inputlen)
    {
        printf("[FAILED TEST] Failed decompress test! different result output length\n");
        printf("[ PROGNOSIS ] inputlen: %d  outputlen: %d\n", inputlen, outputlen);
        return -1;
    }

    for (i = 0; i < outputlen; i++)
    {
        if (inputbuff[i] != outputbuff[i])
        {
            printf("[FAILED TEST] Failed decompress test! non matching buffers\n");
            return -1;
        }
    }
    return 1;
}

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

FILE *demofile = NULL, *outfile = NULL, *mapfile = NULL, *exmapfile = NULL;
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
        exitperror("Error: failed to open demofile, reason");
    if (readdemo(demofile, &DEMO) < 0)
    {
        fprintf(stderr, "Error: failed to parse demo\n");
        exit(EXIT_FAILURE);
    }
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
}

int main(int argc, char *argv[])
{
    inithuff(NULL);

    setinfo("d(emo)edit, a tool for manipulating teeworlds demos");
    setusage("dedit <demo> [OPTIONS]");

    addarg("<demo>", "Sets input demo", setdemo);

    addopt("-r", "--rename", 2, "<id> <name>", "Renames player with id to name", NULL);
    addopt("-s", "--skin", 2, "<id> <name>", "Set skin of player with id to skin", NULL);
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

    if (argc == 1)
    {
        printhelp();
        exit(EXIT_SUCCESS);
    }

    if (getopt("-h", 0))
        printhelp();

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

    if (maparg)
        setmap(maparg);

    if (Infarg)
        printdemo(&DEMO, 1);
    else if (infarg)
        printdemo(&DEMO, 0);

    if (exmarg)
        printf("Info: not implemented extract map\n");

    int i = 0;
    arg *opt;
    while (1)
    {
        opt = getopt("-s", i);
        if (opt == NULL)
            break;
        runsetskin(opt);
        i++;
    }

    i = 0;
    while (1)
    {
        opt = getopt("-r", i);
        if (opt == NULL)
            break;
        runrename(opt);
        i++;
    }

    if (outarg)
        runoutput(outarg);

    exit(EXIT_SUCCESS);

    // Testing
    /*
    inithuff(NULL);

    FILE *fp = fopen("data/huff_test_cases", "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (testcompress(line) < 0)
            exit(EXIT_FAILURE);
        if (testdecompress(line) < 0)
            exit(EXIT_FAILURE);
    }

    fclose(fp);
    free(line);

    exit(EXIT_SUCCESS);
    */
}
