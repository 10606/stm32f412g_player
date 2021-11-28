#include "usb_commands.h"

#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include "playlist_structures.h"

uint32_t calc_need_rd (uint8_t first_byte)
{
    static const uint32_t sizes[2] = 
    {
        sizeof(find_pattern),
        sizeof(position_t)
    };

    if (first_byte < 128) [[likely]]
        return 1;
    else if (static_cast <size_t> (first_byte - 128) < std::extent_v <decltype(sizes)>) 
        return 1 + sizes[first_byte - 128];
    else
        return 1;
}

