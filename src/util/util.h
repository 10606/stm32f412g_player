#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

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

