#ifndef LIGHT_PLALYLIST_H
#define LIGHT_PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"

typedef struct light_playlist 
{
    playlist_header header;
    file_descriptor * fd;
    uint32_t pos;
    song_header song;
} light_playlist;

uint32_t seek_light_playlist (light_playlist * pl, uint32_t pos);
uint32_t next_light_playlist (light_playlist * pl);
uint32_t init_light_playlist (light_playlist * pl, file_descriptor * fd);

#endif

