#ifndef PLAYLIST_STRUCTURES_H
#define PLAYLIST_STRUCTURES_H

#include <stdint.h>
#include <string.h>
#include "char_sizes.h"

typedef struct playlist_header
{
    uint32_t cnt_songs;
    char playlist_name [pl_name_sz];
} playlist_header;

typedef struct song_header
{
    uint32_t path_offset;
    uint32_t path_len;
    char song_name[song_name_sz];
    char group_name[group_name_sz];
} song_header;

#endif

