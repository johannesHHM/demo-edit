/* huffman compresion based on teeworlds/ddnet huffman */
/*{ https://github.com/teeworlds/teeworlds/blob/master/src/engine/shared/huffman.cpp } */
#include "../inc/huffman.h"

#define HUFF_MAX_NODES (HUFF_MAX_SYMBOLS) * 2 - 1
#define HUFF_MAX_SYMBOLS HUFF_EOF_SYMBOL + 1
#define HUFF_EOF_SYMBOL 256

typedef struct
{
    unsigned short symbol;
    unsigned int bits;
    unsigned int numbits;
    unsigned short left, right;
} huffnode;

typedef struct
{
    unsigned short symbol;
    int freq;
} constructnode;

const unsigned int freqtable[HUFF_MAX_SYMBOLS] = {
    1 << 30, 4545, 2657, 431, 1950, 919, 444, 482, 2244, 617, 838, 542, 715, 1814, 304, 240, 754, 212, 647, 186,
    283,     131,  146,  166, 543,  164, 167, 136, 179,  859, 363, 113, 157, 154,  204, 108, 137, 180, 202, 176,
    872,     404,  168,  134, 151,  111, 113, 109, 120,  126, 129, 100, 41,  20,   16,  22,  18,  18,  17,  19,
    16,      37,   13,   21,  362,  166, 99,  78,  95,   88,  81,  70,  83,  284,  91,  187, 77,  68,  52,  68,
    59,      66,   61,   638, 71,   157, 50,  46,  69,   43,  11,  24,  13,  19,   10,  12,  12,  20,  14,  9,
    20,      20,   10,   10,  15,   15,  12,  12,  7,    19,  15,  14,  13,  18,   35,  19,  17,  14,  8,   5,
    15,      17,   9,    15,  14,   18,  8,   10,  2173, 134, 157, 68,  188, 60,   170, 60,  194, 62,  175, 71,
    148,     67,   167,  78,  211,  67,  156, 69,  1674, 90,  174, 53,  147, 89,   181, 51,  174, 63,  163, 80,
    167,     94,   128,  122, 223,  153, 218, 77,  200,  110, 190, 73,  174, 69,   145, 66,  277, 143, 141, 60,
    136,     53,   180,  57,  142,  57,  158, 61,  166,  112, 152, 92,  26,  22,   21,  28,  20,  26,  30,  21,
    32,      27,   20,   17,  23,   21,  30,  22,  22,   21,  27,  25,  17,  27,   23,  18,  39,  26,  15,  21,
    12,      18,   18,   27,  20,   18,  15,  19,  11,   17,  33,  12,  18,  15,   19,  18,  16,  26,  17,  18,
    9,       10,   25,   22,  22,   17,  20,  16,  6,    16,  15,  20,  14,  18,   24,  335, 1517};

huffnode *rootnode;
huffnode nodes[HUFF_MAX_NODES];
int numnodes;

void merge(constructnode **arr, int left, int middle, int right)
{
    int i, j, k;
    int larrlen = middle - left + 1;
    int rarrlen = right - middle;

    constructnode *larr[larrlen], *rarr[rarrlen];

    for (i = 0; i < larrlen; i++)
        larr[i] = arr[left + i];
    for (j = 0; j < rarrlen; j++)
        rarr[j] = arr[middle + 1 + j];

    i = j = 0;
    k = left;

    while (i < larrlen && j < rarrlen)
    {
        if (larr[i]->freq >= rarr[j]->freq)
        {
            arr[k] = larr[i];
            i++;
        }
        else
        {
            arr[k] = rarr[j];
            j++;
        }
        k++;
    }
    while (i < larrlen)
    {
        arr[k] = larr[i];
        i++;
        k++;
    }
    while (j < rarrlen)
    {
        arr[k] = rarr[j];
        j++;
        k++;
    }
}

void mergesort(constructnode **arr, int left, int right)
{
    if (left < right)
    {
        int middle = left + (right - left) / 2;
        mergesort(arr, left, middle);
        mergesort(arr, middle + 1, right);
        merge(arr, left, middle, right);
    }
}

void setbits(huffnode *node, int bits, unsigned int depth)
{
    if (node->right != 0xffff)
        setbits(&nodes[node->right], bits | (1 << depth), depth + 1);
    if (node->left != 0xffff)
        setbits(&nodes[node->left], bits, depth + 1);

    if (node->numbits)
    {
        node->bits = bits;
        node->numbits = depth;
    }
}

void constructhufftree(const unsigned int *frequencies)
{
    constructnode nodesdata[HUFF_MAX_SYMBOLS];
    constructnode *sortednodes[HUFF_MAX_SYMBOLS];

    for (int i = 0; i < HUFF_MAX_SYMBOLS; i++)
    {
        nodes[i].numbits = 0xffffffff;
        nodes[i].symbol = i;
        nodes[i].left = 0xffff;
        nodes[i].right = 0xffff;

        if (i == HUFF_EOF_SYMBOL)
            nodesdata[i].freq = 1;
        else
            nodesdata[i].freq = frequencies[i];
        nodesdata[i].symbol = i;
        sortednodes[i] = &nodesdata[i];
    }

    numnodes = HUFF_MAX_SYMBOLS;

    for (int numnodesleft = HUFF_MAX_SYMBOLS; numnodesleft > 1; numnodesleft--)
    {
        mergesort(sortednodes, 0, numnodesleft - 1);
        nodes[numnodes].numbits = 0;
        nodes[numnodes].left = sortednodes[numnodesleft - 1]->symbol;
        nodes[numnodes].right = sortednodes[numnodesleft - 2]->symbol;
        sortednodes[numnodesleft - 2]->symbol = numnodes;
        sortednodes[numnodesleft - 2]->freq = sortednodes[numnodesleft - 1]->freq + sortednodes[numnodesleft - 2]->freq;
        numnodes++;
    }
    rootnode = &nodes[numnodes - 1];
    setbits(rootnode, 0, 0);
}

void inithuff(const unsigned int *frequencies)
{
    if (!frequencies)
        constructhufftree(freqtable);
    else
        constructhufftree(frequencies);
}

int compresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize)
{
    int outputlen = 0;
    char bits = 0;
    char numbits = 0;
    unsigned short symbol;

    for (int y = 0; y < inputsize; y++)
    {
        symbol = (unsigned char)*(inputbuff + y);
    appendeof:
        for (unsigned int i = 0; i < nodes[symbol].numbits; i++)
        {
            char bit = (nodes[symbol].bits >> i) & 1;
            bits |= (bit << numbits);
            numbits++;
            if (numbits == 8)
            {
                if (outputlen >= outputsize - 1)
                    return -1;
                outputbuff[outputlen] = bits;
                outputlen++;
                numbits = 0;
                bits = 0;
            }
        }
    }
    if (symbol != HUFF_EOF_SYMBOL)
    {
        symbol = HUFF_EOF_SYMBOL;
        goto appendeof;
    }
    outputbuff[outputlen] = bits;
    outputlen++;
    return outputlen;
}

int decompresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize)
{
    int inputoff = 0;
    int outputlen = 0;
    int bits = 0;
    int numbits = 0;

    huffnode *node;
    while (1)
    {
        while (numbits < 24 && inputoff != inputsize)
        {
            bits |= (unsigned char)inputbuff[inputoff++] << numbits;
            numbits += 8;
        }

        node = rootnode;
        while (1)
        {
            if (bits & 1)
                node = &nodes[node->right];
            else
                node = &nodes[node->left];

            bits >>= 1;
            numbits--;

            if (node->numbits)
                break;

            if (numbits == 0)
                return -1;
        }

        if (node->symbol == HUFF_EOF_SYMBOL)
            return outputlen;

        if (outputlen == outputsize)
            return -1;

        outputbuff[outputlen++] = node->symbol;
    }
}
