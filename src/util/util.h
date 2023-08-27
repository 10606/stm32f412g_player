#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

typedef uint32_t ret_code;
inline constexpr const ret_code memory_limit = 101;
inline constexpr const ret_code file_expected_song_found = 201;

char check_near (uint32_t pos_a, uint32_t pos_b, uint32_t max_pos_a, uint32_t view_pos_a, uint32_t border_pos_a); // b in window a
void sprint (char * dst, size_t sz, const char * format, uint32_t value); // dst has size [sz + 1]
void sprint_mod_1000 (char * dst, size_t sz, uint32_t value); // dst has size [sz + 1]

template <uint32_t border_cnt, uint32_t view_cnt>
uint32_t calc_index_set_selected (uint32_t pos, uint32_t cnt, char * selected)
{
    uint32_t index;
    if (pos < border_cnt) // on top
    {
        index = 0;
        selected[pos] |= 1;
    }
    else if (pos + border_cnt >= cnt) // on bottom
    {
        index = cnt - view_cnt;
        selected[pos - index] |= 1;
    }
    else // on middle
    {
        index = pos - border_cnt;
        selected[border_cnt] |= 1;
    }
    return index;
}

#endif

