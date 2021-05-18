#include "util.h"

#include <stdio.h>
#include <string.h>

char check_near (uint32_t pos_a, uint32_t pos_b, uint32_t max_pos_a, uint32_t view_pos_a, uint32_t border_pos_a)
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

void sprint (char * dst, size_t sz, const char * format, uint32_t value) // dst has size [sz + 1]
{
    int wr = snprintf(dst, sz + 1, format, value);
    memset(dst + wr, ' ', sz - wr);
}

void sprint_mod_1000 (char * dst, size_t sz, uint32_t value) // dst has size [sz + 1]
{
    int wr = snprintf(dst, sz + 1, ((value > 999)? "%03lu" : "%3lu"), value % 1000);
    memset(dst + wr, ' ', sz - wr);
}

