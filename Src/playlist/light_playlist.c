#include "light_playlist.h"
#include "playlist_common.h"

uint32_t seek_light_playlist (light_playlist * pl, uint32_t pos)
{
    pos %= pl->header.cnt_songs;
    pl->pos = pos;
    uint32_t ret;
    if ((ret = f_seek(pl->fd, sizeof(playlist_header) + pl->pos * sizeof(song_header))))
        return ret;
    if ((ret = read_song(&pl->song, pl->fd)))
        return ret;
    return 0;
}

uint32_t next_light_playlist (light_playlist * pl)
{
    if (pl->pos + 1 == pl->header.cnt_songs)
        return seek_light_playlist(pl, 0);
    else
        return read_song(&pl->song, pl->fd);
}
    
uint32_t init_light_playlist (light_playlist * pl, file_descriptor * fd)
{
    pl->fd = fd;
    uint32_t ret;
    playlist_header old_header;
    memcpy(&old_header, &pl->header, sizeof(playlist_header));
    if ((ret = f_seek(pl->fd, 0)))
        return ret;
    if ((ret = read_header(&pl->header, pl->fd)))
        return ret;
    if ((ret = seek_light_playlist(pl, 0)))
    {
        memcpy(&pl->header, &old_header, sizeof(playlist_header));
        return ret;
    }
    return 0;
}

