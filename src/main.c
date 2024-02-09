#include "../inc/demo.h"
#include "../inc/huffman.h"
#include "../inc/commands.h"

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

int main()
{
    inithuff(NULL);

    FILE *fp = fopen("data/test.demo", "r");
    FILE *op = fopen("data/out.demo", "w");

    demo d;

    readdemo(fp, &d);
    
    // printdemo(&d, 1);

    int retid = setnamebyid(1, "NamedByID", &d);
    int retname = setnamebyname("New Hero", "NamedByName", &d);
    
    //TODO quirk, when doing further edits keep previous ones in mind
    int retskin = setskinbyname("NamedByName", "bomb", &d);

    printf("id: %d name: %d\n", retid, retname);
    printf("skin: %d\n", retskin);

    writedemo(op, &d);

    fclose(fp);
    fclose(op);

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
