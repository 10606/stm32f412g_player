#include "light_playlist.h"

uint32_t light_playlist::seek (uint32_t _pos)
{
    if (header.cnt_songs == 0)
        return 0;
    _pos %= header.cnt_songs;
    uint32_t ret;
    file_descriptor old_fd = fd;
    if ((ret = fd.seek(sizeof(playlist_header) + _pos * sizeof(song_header))))
        return ret;
    if ((ret = read_song(&song, fd)))
    {
        fd = old_fd;
        return ret;
    }
    pos = _pos;
    return 0;
}

uint32_t light_playlist::next ()
{
    if (header.cnt_songs == 0)
        return 0;
    if (pos + 1 == header.cnt_songs)
        return seek(0);
    else
    {
        uint32_t ret = read_song(&song, fd);
        if (ret)
            return ret;
        pos++;
    }
    return 0;
}
    
void light_playlist::init_base ()
{
    pos = 0;
    memset(song.song_name, ' ', sizeof(song.song_name));
    memset(song.group_name, ' ', sizeof(song.group_name));
    header.cnt_songs = 0;
    memset(header.playlist_name, ' ', sizeof(header.playlist_name));
}

light_playlist::light_playlist () :
    fd()
{
    init_base();
}

uint32_t light_playlist::open_file ()
{
    init_base();
    if (fd.is_fake())
        return 0;
    
    uint32_t ret;
    playlist_header old_header(header);
    if ((ret = fd.seek(0)))
        return ret;
    if ((ret = read_header(&header, fd)))
        return ret;
    if ((ret = seek(0)))
    {
        header = old_header;
        return ret;
    }
    return 0;
}

uint32_t light_playlist::read_song (song_header * song, file_descriptor & fd)
{
    return fd.read_all_fixed((char *)song, sizeof(song_header));
}

uint32_t light_playlist::read_header (playlist_header * header, file_descriptor & fd)
{
    return fd.read_all_fixed((char *)header, sizeof(playlist_header));
}

