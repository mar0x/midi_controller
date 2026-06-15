#pragma once

#include <stdint.h>

namespace patch_mate {

inline void
print_number(char *buf, uint8_t size, unsigned long n)
{
    char *p = buf + size;

    do {
        char c = n % 10;
        n /= 10;

        *--p = c + '0';
    } while (n);

    while (p > buf) { *--p = ' '; }
}

}
