#include "endianness.h"

int is_little_endian(void)
{
    uint32_t magic = 0x00000001;
    uint8_t black_magic = *(uint8_t *)&magic;
    return black_magic;
}

uint32_t reversed(uint32_t val)
{
    return ((val>>24)&0xff) |
           ((val<< 8)&0xff0000) |
           ((val>> 8)&0xff00) |
           ((val<<24)&0xff000000);
}