#ifndef PLAYLIST_STRUCTURES_H
#define PLAYLIST_STRUCTURES_H

#include <stdint.h>
#include <string.h>
#include "char_sizes.h"

struct playlist_header
{
    uint32_t cnt_songs;
    char playlist_name [sz::pl_name];
};

struct song_header
{
    uint32_t path_offset;
    uint32_t path_len;
    char song_name[sz::song_name];
    char group_name[sz::group_name];
};

struct find_pattern
{
    uint32_t song_len;
    uint32_t group_len;
    char song_name[sz::song_name];
    char group_name[sz::group_name];
};

#endif

