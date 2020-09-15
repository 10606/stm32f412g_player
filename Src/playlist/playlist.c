#include "playlist.h"
#include "playlist_common.h"

uint32_t memory_limit = 101;

uint32_t seek_playlist (playlist * pl, uint32_t pos)
{
    pos %= pl->header.cnt_songs;
    pl->pos = pos;
    uint32_t ret;
    {
        if ((ret = f_seek(pl->fd, sizeof(playlist_header) + pl->pos * sizeof(song_header))))
        {
            return ret;
        }
        if ((ret = read_song(&pl->song, pl->fd)))
        {
            return ret;
        }
    }

    if (pl->song.path_len > pl->path_sz)
    {
        if (pl->path)
        {
            free(pl->path);
        }
        pl->path = (char (*)[12])malloc(pl->song.path_len * sizeof(char[12]));
        if (!pl->path)
        {
            pl->path_sz = 0;
            return memory_limit;
        }
        pl->path_sz = pl->song.path_len;
    }
    
    {
        if ((ret = f_seek(pl->fd, pl->song.path_offset)))
        {
            return ret;
        }
        uint32_t rd_sum = 0;
        uint32_t rd;
        while (rd_sum != pl->song.path_len * sizeof(char[12]))
        {
            if ((ret = f_read(pl->fd, (char *)pl->path + rd_sum, pl->song.path_len * sizeof(char[12]) - rd_sum, &rd)))
            {
                return ret;
            }
            rd_sum += rd;
        }
    }
    return 0;
}

uint32_t next_playlist (playlist * pl)
{
    if (pl->pos + 1 == pl->header.cnt_songs)
    {
        return seek_playlist(pl, 0);
    }
    else
    {
        return seek_playlist(pl, pl->pos + 1);
    }
}
    
uint32_t prev_playlist (playlist * pl)
{
    if (pl->pos == 0)
    {
        return seek_playlist(pl, pl->header.cnt_songs - 1);
    }
    else
    {
        return seek_playlist(pl, pl->pos - 1);
    }
}
    
uint32_t init_playlist (playlist * pl, file_descriptor * fd)
{
    pl->fd = fd;
    pl->path = 0;
    pl->path_sz = 0;
    uint32_t ret;
    if ((ret = f_seek(pl->fd, 0)))
    {
        return ret;
    }
    if ((ret = read_header(&pl->header, pl->fd)))
    {
        return ret;
    }
    return seek_playlist(pl, 0);
}

void destroy_playlist (playlist * pl)
{
    if (pl->path)
    {
        free(pl->path);
    }
}

uint32_t open_song (playlist * pl, file_descriptor * fd)
{
    return open(fd, pl->path, pl->song.path_len);
}

