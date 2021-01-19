#include "playlist.h"
#include "playlist_common.h"

uint32_t memory_limit = 101;

uint32_t seek_playlist (playlist * pl, uint32_t pos)
{
    pos %= pl->header.cnt_songs;
    uint32_t ret;
    song_header old_song_header;
    memcpy(&old_song_header, &pl->song, sizeof(song_header));
    {
        ret = f_seek(pl->fd, sizeof(playlist_header) + pos * sizeof(song_header));
        if (ret)
            return ret;
        ret = read_song(&pl->song, pl->fd);
        if (ret)
            return ret;
    }

    if (pl->song.path_len > pl->path_sz)
    {
        char (* tmp)[12] = (char (*)[12])malloc(pl->song.path_len * sizeof(char[12]));
        if (!tmp)
        {
            memcpy(&pl->song, &old_song_header, sizeof(song_header));
            return memory_limit;
        }
        
        if (pl->path)
        {
            memmove(tmp, pl->path, old_song_header.path_len * sizeof(char[12]));
            free(pl->path);
        }
        pl->path = tmp;
        pl->path_sz = pl->song.path_len;
    }
    
    {
        ret = f_seek(pl->fd, pl->song.path_offset);
        if (ret)
        {
            memcpy(&pl->song, &old_song_header, sizeof(song_header));
            return ret;
        }
        uint32_t rd;
        ret = f_read_all_fixed(pl->fd, (char *)pl->path, pl->song.path_len * sizeof(char[12]), &rd);
        if (ret)
        {
            memcpy(&pl->song, &old_song_header, sizeof(song_header));
            return ret;
        }
    }
    pl->pos = pos;
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
    destroy_playlist(pl);
    pl->fd = fd;
    pl->path = 0;
    pl->path_sz = 0;
    uint32_t ret;
    ret = f_seek(pl->fd, 0);
    if (ret) // fseek 0 don't fail
        return ret;

    playlist_header old_header;
    memcpy(&old_header, &pl->header, sizeof(playlist_header));
    ret = read_header(&pl->header, pl->fd);
    if (ret)
        return ret;
    ret = seek_playlist(pl, 0);
    if (ret)
    {
        memcpy(&pl->header, &old_header, sizeof(playlist_header));
        return ret;
    }
    return 0;
}

char (* release_path (playlist * pl)) [12] 
{
    char (* path) [12] = pl->path;
    pl->path = 0;
    pl->path_sz = 0;
    return path;
}

void destroy_playlist (playlist * pl)
{
    if (pl->path)
        free(pl->path);
    pl->path = 0;
}

