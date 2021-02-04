#ifndef PLAYLIST_COMMON_H
#define PLAYLIST_COMMON_H

#include "FAT.h"
#include "playlist_structures.h"

uint32_t read_song (song_header * song, file_descriptor * fd);
uint32_t read_header (playlist_header * song, file_descriptor * fd);

#endif

