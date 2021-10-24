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
        path_backup(nullptr),
        path_sz(0),
        lpl()
    {}

    ~playlist ()
    {
        free(path);
        free(path_backup);
    }
    
    playlist (playlist && other);
    playlist & operator = (playlist && other);
    playlist (playlist const & other) = delete;
    playlist & operator = (playlist const & other) = delete;
    
    // not operator = 
    //  because need ret code
    ret_code clone (playlist const & other) 
    {
        filename_t * new_path = static_cast <filename_t *> (malloc(other.path_sz * sizeof(*path)));
        if (!new_path)
            return memory_limit;
        
        filename_t * new_path_backup = static_cast <filename_t *> (malloc(other.path_sz * sizeof(*path)));
        if (!new_path_backup)
        {
            free(new_path);
            return memory_limit;
        }
        
        free(path);
        free(path_backup);
        path_sz = other.path_sz;
        path = new_path;
        memcpy(path, other.path, other.lpl.song.path_len * sizeof(*path));
        path_backup = new_path_backup;
        lpl = other.lpl;
        return 0;
    }
    
    /// backup.song.path_len <= song.path_len
    ret_code open (light_playlist const & other_lpl, uint32_t pos_selected, playlist const & backup);
    
    ret_code seek (uint32_t new_pos, light_playlist const & backup);
    ret_code next (light_playlist const & backup);
    ret_code prev (light_playlist const & backup);
    
    constexpr void make_fake ()
    {
        lpl.fd.init_fake();
        lpl.init_base();
    }

    constexpr bool is_fake () const
    {
        return lpl.fd.is_fake();
    }


    filename_t * path;
    filename_t * path_backup;
    uint32_t path_sz;
    light_playlist lpl;
    
private:
    ret_code realloc (light_playlist const & old_lpl);
};

#endif

