#include "playlist_common.h"

uint32_t read_song (song_header * song, file_descriptor * fd)
{
    return fd->read_all_fixed((char *)song, sizeof(song_header));
}

uint32_t read_header (playlist_header * header, file_descriptor * fd)
{
    return fd->read_all_fixed((char *)header, sizeof(playlist_header));
}

