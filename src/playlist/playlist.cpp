#include "playlist.h"

#include <utility>
uint32_t memory_limit = 101;

uint32_t playlist::realloc (light_playlist const & old_lpl)
{
    uint32_t new_size = lpl.song.path_len;
    if (new_size > path_sz)
    {
        filename_t * tmp = static_cast <filename_t *> (malloc(new_size * sizeof(*path)));
        if (!tmp)
        {
            lpl = old_lpl;
            return memory_limit;
        }
        
        if (path)
        {
            memmove(tmp, path, old_lpl.song.path_len * sizeof(*path));
            free(path);
        }
        path = tmp;
        path_sz = lpl.song.path_len;
    }
    return 0;
}

uint32_t playlist::seek (uint32_t new_pos)
{
    if (lpl.header.cnt_songs == 0)
        return 0;

    new_pos %= lpl.header.cnt_songs;
    uint32_t ret;
    light_playlist old_lpl = lpl;
    ret = lpl.seek(new_pos);
    if (ret)
        return ret;

    file_descriptor fd(lpl.fd, 0);
    ret = realloc(old_lpl);
    if (ret)
        goto err;
    ret = fd.seek(lpl.song.path_offset);
    if (ret)
        goto err;
    ret = fd.read_all_fixed((char *)path, lpl.song.path_len * sizeof(*path));
    if (ret)
        goto err;
    return 0;

err:
    lpl = old_lpl;
    return ret;
}

uint32_t playlist::next ()
{
    if (lpl.pos + 1 == lpl.header.cnt_songs)
        return seek(0);
    else
        return seek(lpl.pos + 1);
}
    
uint32_t playlist::prev ()
{
    if (lpl.pos == 0)
        return seek(lpl.header.cnt_songs - 1);
    else
        return seek(lpl.pos - 1);
}
    
uint32_t playlist::open (light_playlist const & other_lpl, uint32_t pos_selected)
{
    uint32_t ret;
    playlist old_pl(std::move(*this));
    lpl.fd.copy_seek_0(other_lpl.fd);
    lpl.init_base();
    if (lpl.fd.is_fake())
        return 0;
    
    lpl = other_lpl;
    if ((ret = seek(pos_selected)) == 0)
        return 0;

    *this = std::move(old_pl);
    return ret;
}

playlist::playlist (playlist && src) : 
    path(src.path),
    path_sz(src.path_sz),
    lpl(std::move(src.lpl))
{
    src.path = nullptr;
    src.path_sz = 0;
    src.lpl.fd.init_fake();
}

playlist & playlist::operator = (playlist && src)
{
    lpl = std::move(src.lpl);
    src.lpl.fd.init_fake();
    src.lpl.pos = 0;
    
    path_sz = src.path_sz;
    src.path_sz = 0;
    
    free(path);
    path = src.path;
    src.path = nullptr;
    
    return *this;
}

