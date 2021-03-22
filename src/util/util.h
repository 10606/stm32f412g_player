#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

inline uint32_t add_in_bound (uint32_t value, uint32_t a, uint32_t b, uint32_t add)
{
    if (value + add < value)
        return b;
    if (value + add > b)
        return b;
    return value + add;
}

inline uint32_t sub_in_bound (uint32_t value, uint32_t a, uint32_t b, uint32_t add)
{
    if (value < add)
        return a;
    if (value - add < a)
        return a;
    return value - add;
}

inline int32_t nearest_to_zero (int32_t a, int32_t b)
{
    if (abs(a) < abs(b))
        return a;
    else
        return b;
}

inline char check_near (uint32_t pos_a, uint32_t pos_b, uint32_t max_pos_a, uint32_t view_pos_a, uint32_t border_pos_a)
{
    if (max_pos_a <= view_pos_a)
        return 1;

    if (pos_a < border_pos_a) // on top
        return pos_b < view_pos_a;
    else if (pos_a + border_pos_a >= max_pos_a) // on bottom
        return pos_b >= max_pos_a - view_pos_a;
    else                                        // on middle
        return  (pos_b >= pos_a - border_pos_a) &&
                (pos_b <= pos_a + border_pos_a);
}

#endif

