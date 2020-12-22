#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

inline uint32_t min (uint32_t a, uint32_t b)
{
    return (a < b)? a : b;
}

inline uint32_t max (uint32_t a, uint32_t b)
{
    return (a > b)? a : b;
}

#endif

