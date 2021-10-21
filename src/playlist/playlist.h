#ifndef PLALYLIST_H
#define PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"
#include "light_playlist.h"

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
    
    uint32_t open (light_playlist const & other_lpl, uint32_t pos_selected);
    uint32_t seek (uint32_t pos);
    uint32_t next ();
    uint32_t prev ();
    
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
    uint32_t realloc (light_playlist const & old_lpl);
};

extern uint32_t memory_limit;

#endif

