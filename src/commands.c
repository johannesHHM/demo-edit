#include "../inc/demo.h"
#include "../inc/pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO set by id and name has much repeating, fix that
int setnamebyid(int id, char *new, demo *demo)
{
    int newlen = strlen(new);
    if (newlen >= 15)
        return -1;

    int intname[4] = {0x80808080, 0x80808080, 0x80808080, 0x80808000};
    strtoint(new, newlen + 1, intname);
    int found = 0;

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        if (demo->data.chunks[i].type != DEMOSNAP)
            continue;
        demosnap *snap = demo->data.chunks[i].data.snap;

        for (int y = 0; y < snap->numitems; y++)
        {
            demosnapitem *item = &snap->items[y];
            if (item->type == 11 && item->id == id)
            {
                memcpy(item->data, intname, 4 * sizeof(int));
                found++;
            }
        }
    }
    return found;
}

int setnamebyname(char *old, char *new, demo *demo)
{
    int newlen = strlen(new);
    if (newlen >= 15)
        return -1;

    int intname[4] = {0x80808080, 0x80808080, 0x80808080, 0x80808000};
    strtoint(new, newlen + 1, intname);
    char found = 0;

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        if (demo->data.chunks[i].type != DEMOSNAP)
            continue;
        demosnap *snap = demo->data.chunks[i].data.snap;

        for (int y = 0; y < snap->numitems; y++)
        {
            demosnapitem *item = &snap->items[y];
            if (item->type != 11)
                continue;

            char name[16];
            intstostr(item->data, 4, name);
            if (strcmp(old, name) == 0)
            {
                memcpy(item->data, intname, 4 * sizeof(int));
                found++;
            }
        }
    }
    return found;
}

int setskinbyid(int id, char *skin, demo *demo)
{
    int skinlen = strlen(skin);
    if (skinlen >= 23)
        return -1;

    int intskin[5] = {0x80808080, 0x80808080, 0x80808080, 0x80808080, 0x80808000};
    strtoint(skin, skinlen + 1, intskin);
    char found = 0;

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        if (demo->data.chunks[i].type != DEMOSNAP)
            continue;
        demosnap *snap = demo->data.chunks[i].data.snap;

        for (int y = 0; y < snap->numitems; y++)
        {
            demosnapitem *item = &snap->items[y];
            if (item->type == 11 && item->id == id)
            {
                memcpy(&item->data[8], intskin, 5 * sizeof(int));
                found++;
            }
        }
    }
    return found;
}

int setskinbyname(char *name, char *skin, demo *demo)
{
    int skinlen = strlen(skin);
    if (skinlen >= 23)
        return -1;

    int intskin[5] = {0x80808080, 0x80808080, 0x80808080, 0x80808080, 0x80808000};
    strtoint(skin, skinlen + 1, intskin);
    char found = 0;

    for (int i = 0; i < demo->data.numchunks; i++)
    {
        if (demo->data.chunks[i].type != DEMOSNAP)
            continue;
        demosnap *snap = demo->data.chunks[i].data.snap;

        for (int y = 0; y < snap->numitems; y++)
        {
            demosnapitem *item = &snap->items[y];
            if (item->type != 11)
                continue;

            char iname[16];
            intstostr(item->data, 4, iname);
            printf("comparing: %s : %s\n", name, iname);
            if (strcmp(name, iname) == 0)
            {
                memcpy(&item->data[8], intskin, 5 * sizeof(int));
                found++;
            }
        }
    }
    return found;
}

int changemap(FILE *map, char *mapname, demo *demo)
{
    int namelen = strlen(mapname);
    if (namelen >= 31)
        return -1;

    fseek(map, 0, SEEK_END);
    long mapsize = ftell(map);
    fseek(map, 0, SEEK_SET);

    char *mapbuff = (char *)malloc(mapsize);
    if (mapbuff == NULL)
        return -1;

    fread(mapbuff, 1, mapsize, map);
    
    if (memcmp(mapbuff, "DATA", 4) != 0)
    {
        free(mapbuff);
        return -1;
    }

    free(demo->map.data);
    demo->map.data = mapbuff;
    
    if (demo->header.version >= 6)
        demo->header.version = 5;

    demo->header.mapsize = mapsize;
    demo->header.mapcrc = mapsize;
    memcpy(demo->header.mapname, mapname, namelen + 1);

    return 1;
}
