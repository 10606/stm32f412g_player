#include "light_playlist.h"
#include "playlist_common.h"

uint32_t light_playlist::seek (uint32_t _pos)
{
    if (header.cnt_songs == 0)
        return 0;
    _pos %= header.cnt_songs;
    pos = _pos;
    uint32_t ret;
    if ((ret = fd.seek(sizeof(playlist_header) + pos * sizeof(song_header))))
        return ret;
    if ((ret = read_song(&song, &fd)))
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
        return read_song(&song, &fd);
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
    playlist_header old_header;
    memcpy(&old_header, &header, sizeof(playlist_header));
    if ((ret = fd.seek(0)))
        return ret;
    if ((ret = read_header(&header, &fd)))
        return ret;
    if ((ret = seek(0)))
    {
        memcpy(&header, &old_header, sizeof(playlist_header));
        return ret;
    }
    return 0;
}

void light_playlist::copy (light_playlist const & other)
{
    memcpy(&header, &other.header, sizeof(header));
    memcpy(&song, &other.song, sizeof(song));
    fd = other.fd;
    pos = other.pos;
}

light_playlist::light_playlist (light_playlist const & other)
{
    copy(other);
}

light_playlist & light_playlist::operator = (light_playlist const & other)
{
    copy(other);
    return *this;
}

