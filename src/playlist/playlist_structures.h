#ifndef PLAYLIST_STRUCTURES_H
#define PLAYLIST_STRUCTURES_H

#include <stdint.h>
#include <string.h>
#include "char_sizes.h"

typedef struct playlist_header
{
    uint32_t cnt_songs;
    char playlist_name [sz::pl_name];
} playlist_header;

typedef struct song_header
{
    uint32_t path_offset;
    uint32_t path_len;
    char song_name[sz::song_name];
    char group_name[sz::group_name];
} song_header;

#endif

