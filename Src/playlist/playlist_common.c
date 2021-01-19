#include "playlist_common.h"

uint32_t read_song (song_header * song, file_descriptor * fd)
{
    uint32_t br;
    return f_read_all_fixed(fd, (char *)song, sizeof(song_header), &br);
}

uint32_t read_header (playlist_header * header, file_descriptor * fd)
{
    uint32_t br;
    return f_read_all_fixed(fd, (char *)header, sizeof(playlist_header), &br);
}

