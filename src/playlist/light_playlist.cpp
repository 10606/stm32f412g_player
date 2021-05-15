#include "light_playlist.h"

uint32_t light_playlist::seek (uint32_t _pos)
{
    if (header.cnt_songs == 0)
        return 0;
    _pos %= header.cnt_songs;
    pos = _pos;
    uint32_t ret;
    if ((ret = fd.seek(sizeof(playlist_header) + pos * sizeof(song_header))))
        return ret;
    if ((ret = read_song(&song, fd)))
        return ret;
    return 0;
}

uint32_t light_playlist::next ()
{
    if (header.cnt_songs == 0)
        return 0;
    if (pos + 1 == header.cnt_songs)
        return seek(0);
    else
        return read_song(&song, fd);
}
    
void light_playlist::init_base ()
{
    pos = 0;
    memset(song.song_name, ' ', song_name_sz);
    memset(song.group_name, ' ', group_name_sz);
    header.cnt_songs = 0;
    memset(header.playlist_name, ' ', pl_name_sz);
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

