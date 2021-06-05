#ifndef DISPLAY_OFFSETS_H
#define DISPLAY_OFFSETS_H

#include <stdint.h>

namespace display
{

struct offsets
{
    static const uint32_t in_line = 12;
    static const uint32_t line = 24;
    static const uint32_t list = 68;
    static const uint32_t song_name = 7;
    static const uint32_t in_song_name = 20;
    static const uint32_t time = 49;
    static const uint32_t x_padding = 4;

    static const uint32_t picture = 65;
    static const uint32_t headband = 65;
};

}

#endif

