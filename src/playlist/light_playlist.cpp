#include "light_playlist.h"

ret_code light_playlist::seek (uint32_t _pos, file_descriptor const & backup)
{
    if (header.cnt_songs == 0)
        return 0;
    _pos %= header.cnt_songs;
    ret_code ret;
    if ((ret = fd.seek(sizeof(playlist_header) + _pos * sizeof(song_header))))
        return ret;
    if ((ret = read_song()))
    {
        fd = backup;
        return ret;
    }
    pos = _pos;
    return 0;
}

ret_code light_playlist::seek (uint32_t _pos)
{
    file_descriptor old_fd = fd;
    return seek(_pos, old_fd);
}

ret_code light_playlist::next_simple ()
{
    ret_code ret = read_song();
    if (ret)
        return ret;
    pos++;
    return 0;
}

ret_code light_playlist::next ()
{
    if (header.cnt_songs == 0)
        return 0;
    if (pos + 1 == header.cnt_songs)
        return seek(0);
    else
        return next_simple();
}
    
ret_code light_playlist::next (file_descriptor const & backup)
{
    if (header.cnt_songs == 0)
        return 0;
    if (pos + 1 == header.cnt_songs)
        return seek(0, backup);
    else
        return next_simple();
}
    
ret_code light_playlist::open_file ()
{
    init_base();
    if (fd.is_fake())
        return 0;
    
    ret_code ret;
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

