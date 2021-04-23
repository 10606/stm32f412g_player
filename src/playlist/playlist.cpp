#include "playlist.h"
#include "playlist_common.h"

#include <utility>
uint32_t memory_limit = 101;

uint32_t playlist::seek (uint32_t new_pos)
{
    if (header.cnt_songs == 0)
        return 0;

    new_pos %= header.cnt_songs;
    uint32_t ret;
    song_header old_song_header;
    memcpy(&old_song_header, &song, sizeof(song_header));
    {
        ret = f_seek(&fd, sizeof(playlist_header) + new_pos * sizeof(song_header));
        if (ret)
            return ret;
        ret = read_song(&song, &fd);
        if (ret)
            return ret;
    }

    if (song.path_len > path_sz)
    {
        char (* tmp)[12] = (char (*)[12])malloc(song.path_len * sizeof(char[12]));
        if (!tmp)
        {
            memcpy(&song, &old_song_header, sizeof(song_header));
            return memory_limit;
        }
        
        if (path)
        {
            memmove(tmp, path, old_song_header.path_len * sizeof(char[12]));
            free(path);
        }
        path = tmp;
        path_sz = song.path_len;
    }
    
    {
        ret = f_seek(&fd, song.path_offset);
        if (ret)
        {
            memcpy(&song, &old_song_header, sizeof(song_header));
            return ret;
        }
        uint32_t rd;
        ret = f_read_all_fixed(&fd, (char *)path, song.path_len * sizeof(char[12]), &rd);
        if (ret)
        {
            memcpy(&song, &old_song_header, sizeof(song_header));
            return ret;
        }
    }
    pos = new_pos;
    return 0;
}

uint32_t playlist::next ()
{
    if (pos + 1 == header.cnt_songs)
        return seek(0);
    else
        return seek(pos + 1);
}
    
uint32_t playlist::prev ()
{
    if (pos == 0)
        return seek(header.cnt_songs - 1);
    else
        return seek(pos - 1);
}
    
void playlist::init_base ()
{
    path = nullptr;
    path_sz = 0;
    pos = 0;
    memset(song.song_name, ' ', song_name_sz);
    memset(song.group_name, ' ', group_name_sz);
    header.cnt_songs = 0;
    memset(header.playlist_name, ' ', pl_name_sz);
}
 
uint32_t playlist::init ()
{
    if (fd.is_fake())
        return 0;
    
    uint32_t ret;
    ret = f_seek(&fd, 0);
    if (ret) // fseek 0 don't fail
        return ret;

    playlist_header old_header;
    memcpy(&old_header, &header, sizeof(playlist_header));
    ret = read_header(&header, &fd);
    if (ret)
        return ret;
    ret = seek(0);
    if (ret)
    {
        memcpy(&header, &old_header, sizeof(playlist_header));
        return ret;
    }
    return 0;
}

playlist::playlist () :
    fd()
{
    init_base();
}

uint32_t playlist::set_file (file_descriptor * _fd, uint32_t pos_selected)
{
    uint32_t ret;
    playlist old_pl(std::move(*this));
    fd.copy_seek_0(*_fd);
    init_base();
    if ((ret = init()) == 0)
    {
        if ((ret = seek(pos_selected)) == 0)
            return 0;
    }

    *this = std::move(old_pl);
    return ret;
}

void playlist::move (playlist && other)
{
    fd.copy(other.fd);
    other.fd.init_fake();
    
    memmove(&header, &other.header, sizeof(playlist_header));
    memmove(&song, &other.song, sizeof(song_header));
}

playlist::playlist (playlist && src)
{
    move(std::move(src));
    pos = src.pos;
    path_sz = src.path_sz;
    path = src.path;
    
    src.path = nullptr;
    src.path_sz = 0;
}

playlist & playlist::operator = (playlist && src)
{
    move(std::move(src));
    std::swap(pos, src.pos);
    std::swap(path_sz, src.path_sz);
    std::swap(path, src.path);
    return *this;
}

void playlist::make_fake ()
{
    free(path);
    fd.init_fake();
    init_base();
}

playlist::~playlist ()
{
    free(path);
    path = 0;
}

bool playlist::is_fake ()
{
    return fd.is_fake();
}

