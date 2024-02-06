#ifndef PACK_H
#define PACK_H

#include <stdio.h>

/* reads a teeworlds packed int from given buffer */
/* the given pointer is incremented */
int readint(unsigned char **cp);

#endif // PACK_H
