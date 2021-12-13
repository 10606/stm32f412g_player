#ifndef DISPLAY_OFFSETS_H
#define DISPLAY_OFFSETS_H

#include <stdint.h>
#include <utility>

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
    static const uint32_t x_time = 0;
    static const uint32_t x_status = 180;
    static const uint32_t x_padding = 4;

    static const uint32_t picture = 65;
    static const uint32_t headband = 65;

    static const constexpr std::pair <uint32_t, uint32_t> bg_color_on_picture = {208, 6};
};

}

#endif

