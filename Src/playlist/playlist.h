#ifndef PLALYLIST_H
#define PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"

typedef struct playlist 
{
    playlist_header header;
    file_descriptor * fd;
    uint32_t pos;
    song_header song;
    uint32_t path_sz;
    char (*path)[12];
} playlist;

extern uint32_t memory_limit;

uint32_t init_playlist (playlist * pl, file_descriptor * fd);
uint32_t seek_playlist (playlist * pl, uint32_t pos);
uint32_t next_playlist (playlist * pl);
uint32_t prev_playlist (playlist * pl);
uint32_t open_song (playlist * pl, file_descriptor * fd);
void destroy_playlist (playlist * pl);

#endif

