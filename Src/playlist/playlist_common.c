#include "playlist_common.h"

uint32_t read_song (song_header * song, file_descriptor * fd)
{
    uint32_t ret;
    uint32_t br_sum = 0;
    uint32_t br;
    while (br_sum != sizeof(song_header))
    {
        if ((ret = f_read(fd, (char *)song + br_sum, sizeof(song_header) - br_sum, &br)))
        {
            return ret;
        }
        br_sum += br;
    }
    return 0;
}

uint32_t read_header (playlist_header * header, file_descriptor * fd)
{
    uint32_t ret;
    uint32_t br_sum = 0;
    uint32_t br;
    while (br_sum != sizeof(playlist_header))
    {
        if ((ret = f_read(fd, (char *)header + br_sum, sizeof(playlist_header) - br_sum, &br)))
        {
            return ret;
        }
        br_sum += br;
    }
    return 0;
}

