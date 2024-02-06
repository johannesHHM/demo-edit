#include <stdio.h>

int reverseint(const unsigned char in[4])
{
    return (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
}

int readint(unsigned char **cp)
{
    int result = 0;
    int len = 1;

    unsigned char src = **cp;

    (*cp)++;
    int sign = (src >> 6) & 1;

    result |= (src & 0b00111111);
    for (int i = 0; i < 4; i++)
    {
        // printf("(%d, ", src);
        if ((src & 0b10000000) == 0)
            break;

        src = **cp;
        (*cp)++;
        len++;

        if (i == 3 && (src & 0b11110000) != 0)
            printf("[ WARNING ] Non zero int padding!\n");

        result |= (src & 0b01111111) << (6 + 7 * i);
    }
    if (len > 1 && src == 0b00000000)
        printf("[ WARNING ] Overlong int encoding!\n");

    result ^= -sign;

    return result;
}
