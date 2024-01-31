#include "../inc/demo.h"

#include <stdio.h>

int reverseint(const unsigned char in[4])
{
    return (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
}

int readdemoheader(FILE *fp, demoheader *dh)
{
    if (fscanf(fp,"TWDEMO%*c%c", &dh->version) != 1)
        return -1;
    if (fscanf(fp, "%64c", dh->netversion) != 1)
        return -2;
    if (fscanf(fp, "%64c", dh->mapname) != 1)
        return -3;
    if (fread(dh->mapsize, 1, 4, fp) != 4)
        return -4;
    if (fread(dh->mapcrc, 1, 4, fp) != 4)
        return -5;
    if (fscanf(fp, "%8c", dh->type) != 1)
        return -6;
    if (fread(dh->length, 1, 4, fp) != 4)
        return -7;
    if (fscanf(fp, "%20c", dh->timestamp) != 1)
        return -8;
    printf("version: %hhu\n", dh->version);
    printf("net_version: %s\n", dh->netversion);
    printf("mapname: %s\n", dh->mapname);
    printf("mapsize: %i\n", reverseint(dh->mapsize));
    printf("mapcrc: %i\n", reverseint(dh->mapcrc));
    printf("type: %s\n", dh->type);
    printf("length: %i\n", reverseint(dh->length));
    printf("timestamp: %s\n", dh->timestamp);
    return 1;
}

int readdemotimeline(FILE *fp, demotimeline *dt)
{
    if (fscanf(fp, "%260c", dt->data) != 1)
        return -1;
    return 1;
}