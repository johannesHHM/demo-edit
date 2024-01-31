#ifndef HUFFMAN_H
#define HUFFMAN_H

/* Initialize huffman */
/* Uses the standard frequency table if NULL is provided */
void inithuff(const unsigned int *frequencies);

/* Compress contents of input buffer, result in output buffer */
/* Returns -1 on too small output buffer */
int compresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize);

/* Decompress contents of input buffer, result in output buffer */
/* Returns -1 on decompress error */
int decompresshuff(const char *inputbuff, int inputsize, char *outputbuff, int outputsize);

#endif // HUFFMAN_H
