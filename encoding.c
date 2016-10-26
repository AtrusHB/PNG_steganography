#include "encoding.h"

int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1; //bitshift assignment
        base *= base;
    }

    return result;
}

void writebit(unsigned long bitholder, unsigned char *byte, int bitposition)
{
    //printf("%02X\n", *byte);
    if (bitholder & ipow(2, bitposition))
        *byte |= 1;
    else
        *byte &= 0xFE;
}