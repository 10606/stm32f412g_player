#ifndef PLALYLIST_H
#define PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"
#include "light_playlist.h"
#include "util.h"

struct playlist 
{
    constexpr playlist () :
        path(nullptr),
        path_sz(0),
        lpl()
    {}

    ~playlist ()
    {
        free(path);
    }
    
    playlist (playlist && other);
    playlist & operator = (playlist && other);
    playlist (playlist const & other) = delete;
    playlist & operator = (playlist const & other) = delete;

    constexpr void swap (playlist & other)
    {
        std::swap(path, other.path);
        std::swap(path_sz, other.path_sz);
        lpl.swap(other.lpl);
    }

    constexpr bool operator == (playlist const & other) const
    {
        return (lpl.fd == other.lpl.fd) && (lpl.pos == other.lpl.pos);
    }
    
    constexpr bool operator != (playlist const & other) const
    {
        return !operator == (other);
    }
    
    // not operator = 
    //  because need ret code
    ret_code clone (playlist const & other) 
    {
        if (path_sz < other.path_sz)
        {
            filename_t * new_path = static_cast <filename_t *> (malloc(other.path_sz * sizeof(*path)));
            if (!new_path)
                return memory_limit;
            
            free(path);
            path_sz = other.path_sz;
            path = new_path;
        }
        
        memcpy(path, other.path, other.lpl.song.path_len * sizeof(*path));
        lpl = other.lpl;
        return 0;
    }
    
    // backup.lpl.song.path_len <= song.path_len
    ret_code open (light_playlist const & other_lpl, uint32_t pos_selected, playlist const & backup);
    
    ret_code seek (uint32_t new_pos, playlist const & backup);
    ret_code next (playlist const & backup);
    ret_code prev (playlist const & backup);
    
    constexpr void make_fake ()
    {
        lpl.make_fake();
    }

    constexpr bool is_fake () const
    {
        return lpl.fd.is_fake();
    }


    filename_t * path;
    uint32_t path_sz;
    light_playlist lpl;
    
private:
    ret_code realloc (light_playlist const & old_lpl);
};

#endif

