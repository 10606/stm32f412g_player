#include "playlist.h"

#include <utility>

ret_code playlist::realloc (light_playlist const & old_lpl)
{
    uint32_t new_size = lpl.song.path_len;
    if (new_size > path_sz)
    {
        filename_t * new_path = static_cast <filename_t *> (::realloc(path, new_size * sizeof(*path)));
        if (!new_path)
        {
            lpl = old_lpl;
            return memory_limit;
        }
        path = new_path;
        path_sz = lpl.song.path_len;
    }
    return 0;
}

ret_code playlist::seek (uint32_t new_pos, playlist const & backup)
{
    if (lpl.header.cnt_songs == 0)
        return 0;

    new_pos %= lpl.header.cnt_songs;
    ret_code ret;
    
    ret = lpl.seek(new_pos, lpl.fd);
    if (ret)
    {
        lpl = backup.lpl;
        return ret;
    }

    file_descriptor fd(lpl.fd, 0);
    ret = realloc(backup.lpl);
    if (ret)
        goto err;
    ret = fd.seek(lpl.song.path_offset);
    if (ret)
        goto err;
    ret = fd.read_all_fixed((char *)path, lpl.song.path_len * sizeof(*path));
    if (ret)
    {
        memcpy(path, backup.path, backup.lpl.song.path_len * sizeof(*path));
        goto err;
    }
    return 0;

err:
    lpl = backup.lpl;
    return ret;
}

ret_code playlist::next (playlist const & backup)
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    if (lpl.pos + 1 == lpl.header.cnt_songs)
        return seek(0, backup);
    else
        return seek(lpl.pos + 1, backup);
}
    
ret_code playlist::prev (playlist const & backup)
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    if (lpl.pos == 0)
        return seek(lpl.header.cnt_songs - 1, backup);
    else
        return seek(lpl.pos - 1, backup);
}
    
ret_code playlist::open (light_playlist const & other_lpl, uint32_t pos_selected, playlist const & backup)
{
    lpl = other_lpl;
    ret_code ret = seek(pos_selected, backup);
    if (!ret)
        return 0;
    if (backup.lpl.header.cnt_songs != 0)
        memcpy(path, backup.path, backup.lpl.song.path_len * sizeof(*path));
    return ret;
}

playlist::playlist (playlist && src) : 
    path(src.path),
    path_sz(src.path_sz),
    lpl(std::move(src.lpl))
{
    src.path = nullptr;
    src.path_sz = 0;
    src.lpl.make_fake();
}

playlist & playlist::operator = (playlist && src)
{
    lpl = std::move(src.lpl);
    src.lpl.make_fake();
    src.lpl.pos = 0;
    
    path_sz = src.path_sz;
    src.path_sz = 0;
    
    free(path);
    path = src.path;
    src.path = nullptr;
    
    return *this;
}

