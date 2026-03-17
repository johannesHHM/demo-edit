#include "../inc/chat.h"
#include "../inc/demo.h"
#include "../inc/huffman.h"
#include "../inc/pack.h"
#include <stdio.h>
#include <string.h>

#define GAME_MAX_CLIENTS 512
#define GAME_TICK_RATE 50

typedef struct
{
    int id;
    char name[16];
    int intname[4];
} client;

typedef struct
{
    int firsttick;
    int tick;
    client clients[GAME_MAX_CLIENTS];
} game;

void resetclients(game *g)
{
    int i;

    for (i = 0; i < GAME_MAX_CLIENTS; i++)
    {
        g->clients[i].id = -1;
        memset(g->clients[i].name, 0, 16);
        memset(g->clients[i].intname, 0, 16);
    }
}

void initgame(game *g)
{
    g->firsttick = -1;
    g->tick = -1;
    resetclients(g);
}

int ticktosecond(int firsttick, int tick)
{
    return (tick - firsttick) / GAME_TICK_RATE;
}

void gameapplytick(game *g, demotick *t)
{
    if (t->keyframe)
        g->tick = t->delta;

    if (t->innline)
        g->tick += t->delta;

    // NOTE: what does it mean if first tick is not a keyframe?
    if (g->firsttick == -1 && t->keyframe)
        g->firsttick = t->delta;
}

void gameapplysnap(game *g, demosnap *s)
{
    int i;
    demosnapitem *item;

    resetclients(g);
    for (i = 0; i < s->numitems; i++)
    {
        item = &s->items[i];
        if (item->type == 11) // TODO: 0.6 specific
        {
            g->clients[item->id].id = item->id;
            intstostr(item->data, 4, g->clients[item->id].name);
            memcpy(g->clients[item->id].intname, item->data, 16);
        }
    }
}

void gameapplydelta(game *g, demodelta *d)
{
    int i, key;
    short type, id;
    demodeltaitem *item;

    for (i = 0; i < d->numremoveditems; i++)
    {
        key = d->removeditemkeys[i];
        type = (key >> 16) & 0xffff;
        id = (key & 0xffff);

        if (type == 11) // TODO: 0.6 specific
        {
            g->clients[id].id = -1;
            memset(g->clients[id].name, 0, 16);
        }
    }
    for (i = 0; i < d->numitemdeltas; i++)
    {
        item = &d->itemdeltas[i];
        if (item->type == 11) // TODO: 0.6 specific
        {
            if (g->clients[item->id].id == -1)
            {
                g->clients[item->id].id = item->id;
                intstostr(item->data, 4, g->clients[item->id].name);
            }
            else
            {
                for (int i = 0; i < 4; i++)
                    g->clients[item->id].intname[i] += item->data[i];

                intstostr(g->clients[item->id].intname, 4, g->clients[item->id].name);
            }
        }
    }
}

void gameprintchatmessage(game *g, demomessage *m)
{
    // TODO: magic numbs
    char unpacked[1024 * 8];
    char raw[1024 * 8];
    int rawlen, n, clientid, seconds, min, sec;
    char *unpackedptr;
    char *rawptr;

    // TODO: handle error
    decompresshuff(m->data, m->datasize, (char *)unpacked, 1024 * 8);

    unpackedptr = unpacked;
    rawptr = raw;
    rawlen = 0;

    while (*unpackedptr)
    {
        n = readint(&unpackedptr);

        memcpy(rawptr, &n, 4);
        rawptr += 4;
        rawlen += 4;
    }
    *(rawptr) = 0;

    rawptr = raw;

    int id = readint(&rawptr);
    char sys = (id & 1) != 0;
    int msg = id >> 1;

    if (!sys && msg == 3)
    {
        readint(&rawptr);
        clientid = readint(&rawptr);
        seconds = ticktosecond(g->firsttick, g->tick);
        min = seconds / 60;
        sec = seconds % 60;

        if (clientid != -1)
        {
            if (g->clients[clientid].id == -1)
            {
                printf("ERROR, message from non-existent client recieved!\n");
            }
            printf("[%02d:%02d] chat: %s: %s\n", min, sec, g->clients[clientid].name, rawptr);
        }
        else
        {
            printf("[%02d:%02d] chat: *** %s\n", min, sec, rawptr);
        }
    }
}

void printchat(demo *demo)
{
    int i;
    game g;
    demochunk *c;

    initgame(&g);
    for (int i = 0; i < demo->data.numchunks; i++)
    {
        c = &demo->data.chunks[i];
        switch (c->type)
        {
        case DEMOTICK:
            gameapplytick(&g, c->data.tick);
            break;
        case DEMOSNAP:
            gameapplysnap(&g, c->data.snap);
            break;
        case DEMODELTA:
            gameapplydelta(&g, c->data.delta);
            break;
        case DEMOMESSAGE:
            gameprintchatmessage(&g, c->data.message);
            break;
        default:
            printf("Error: unknown chunk found\n");
            break;
        }
    }
}
