#include "../inc/args.h"
#include "../inc/commands.h"
#include "../inc/demo.h"
#include "../inc/huffman.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

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

void run(input *in)
{
    if (!in->demopath)
        exit(EXIT_FAILURE);

    FILE *demofile, *outfile, *mapfile, *exmapfile;
    demofile = NULL;
    outfile = NULL;
    mapfile = NULL;
    exmapfile = NULL;

    demofile = fopen(in->demopath, "r");
    if (!demofile)
    {
        perror("ERROR: Failed to open demo file, reason");
        exit(EXIT_FAILURE);
    }

    if (in->outpath)
    {
        outfile = fopen(in->outpath, "w");
        if (!outfile)
        {
            perror("ERROR: Failed to open output file, reason");
            exit(EXIT_FAILURE);
        }
    }

    if (in->mappath)
    {
        mapfile = fopen(in->mappath, "r");
        if (!mapfile)
        {
            perror("ERROR: Failed to open map file, reason");
            exit(EXIT_FAILURE);
        }
    }

    if (in->extractmap)
    {
        exmapfile = fopen(in->extractmap, "r");
        if (!exmapfile)
        {
            perror("ERROR: Failed to open extract map file, reason");
            exit(EXIT_FAILURE);
        }
        // TODO extract map here
    }

    demo dmo;

    if (readdemo(demofile, &dmo) < 0)
    {
        fprintf(stderr, "ERROR: Failed to parse demo\n");
        exit(EXIT_FAILURE);
    }

    if (in->print)
        printdemo(&dmo, 0);

    if (mapfile)
    {
        char mname[32];
        if (mapname(in->mappath, mname))
        {
            if (changemap(mapfile, mname, &dmo) < 0)
            {
                fprintf(stderr, "ERROR: Failed to set map '%s'\n", in->mappath);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Given map has too long name (must be < 32)\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < in->numcmds; i++)
    {
        runcommand(&in->commands[i], &dmo);
    }

    if (outfile)
    {
        if (writedemo(outfile, &dmo) < 0)
        {
            fprintf(stderr, "ERROR: Failed to write demo to '%s'\n", in->outpath);
            exit(EXIT_FAILURE);
        }
    }

    if (demofile)
        freedemo(&dmo);

}

void printhelp()
{
    printf("HELP! I hope this helped you.\n");
}

int main(int argc, char *argv[])
{
    inithuff(NULL);

    if (argc == 1)
    {
        printhelp();
        exit(EXIT_SUCCESS);
    }

    input in;

    parseargs(&in, argc, argv);

    /*
    printf("demopath: %s\n", in.demopath);
    printf("outpath: %s\n", in.outpath);
    printf("mappath: %s\n", in.mappath);
    printf("extractpath: %s\n", in.extractmap);
    printf("print: %d\n", in.print);

    printf("\ncommands:\n");
    for (int i = 0; i < in.numcmds; i++)
    {
        cmdinput *c = &in.commands[i];
        printf("command: {cmd: %c, type: %c, id: %d, from: %s, to: %s}\n", c->cmdtype, c->settype, c->id, c->from,
    c->to);
    }

    printf("\nEXITING SUCCESS\n");
    */

    run(&in);

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
