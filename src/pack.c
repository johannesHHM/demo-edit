#include <stdio.h>

int frombigendian(const unsigned char in[4])
{
    return (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
}

void tobigendian(const int in, unsigned char out[4])
{
    out[0] = (in >> 24) & 0xff;
    out[1] = (in >> 16) & 0xff;
    out[2] = (in >> 8) & 0xff;
    out[3] = in & 0xff;
}

/* function is lifted from */
/* https://github.com/heinrich5991/libtw2/blob/b9286674da94d3d45b9c10ffce517af394e2d58c/packer/src/lib.rs#L62 */
int readint(char **cp)
{
    int result = 0;
    int len = 1;

    unsigned char src = **cp;
    (*cp)++;

    int sign = ((src >> 6) & 1);
    result |= (src & 0x3f);

    for (int i = 0; i < 4; i++)
    {
        // printf("(%d, ", src);
        if ((src & 0x80) == 0)
            break;

        src = **cp;
        (*cp)++;

        len++;

        if (i == 3 && (src & 0xf0) != 0)
            printf("[ WARNING ] Non zero int padding!\n");

        result |= (src & 0x7f) << (6 + 7 * i);
    }
    if (len > 1 && src == 0x00)
        printf("[ WARNING ] Overlong int encoding!\n");

    result ^= -sign;

    return result;
}

/* function is lifted from */
/* https://github.com/heinrich5991/libtw2/blob/b9286674da94d3d45b9c10ffce517af394e2d58c/packer/src/lib.rs#L105 */
void writeint(int i, char **cp)
{
    int sign = i < 0;
    unsigned int in = i;
    in = (in ^ -sign);
    char next = (in & 0x3f);
    in >>= 6;

    unsigned char head = 0;
    if (in != 0)
        head |= 0x80;
    if (sign != 0)
        head |= 0x40;

    **cp = head | next;
    (*cp)++;

    while (in != 0)
    {
        next = (in & 0x7f);
        in >>= 7;

        if (in != 0)
            head = 0x80;
        else
            head = 0;

        **cp = head | next;
        (*cp)++;
    }
}

/* function is copied from */
/* https://github.com/teeworlds/teeworlds/blob/a1911c8f7d8458fb4076ef8e7651e8ef5e91ab3e/src/game/gamecore.h#L68 */
void intstostr(const int *ints, int num, char *str)
{
    while (num)
    {
        str[0] = (((*ints) >> 24) & 0xff) - 128;
        str[1] = (((*ints) >> 16) & 0xff) - 128;
        str[2] = (((*ints) >> 8) & 0xff) - 128;
        str[3] = ((*ints) & 0xff) - 128;
        str += 4;
        ints++;
        num--;
    }
    str[-1] = 0;
}

void strtoint(const char *str, int num, int *ints)
{
    int intloc = 3;
    *ints = 0;
    while (num)
    {
        if (intloc < 0)
        {
            ints++;
            *ints = 0;
            intloc = 3;
        }
        *ints |= (*str + 128) << (8 * intloc);
        str++;
        intloc--;
        num--;
    }
}
