#ifndef PLALYLIST_H
#define PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"
#include "light_playlist.h"

struct playlist 
{
    playlist ();
    ~playlist ();
    
    playlist (playlist && other);
    playlist & operator = (playlist && other);
    playlist (playlist const & other) = delete;
    playlist & operator = (playlist const & other) = delete;
    
    uint32_t open (light_playlist const & other_lpl, uint32_t pos_selected);
    uint32_t seek (uint32_t pos);
    uint32_t next ();
    uint32_t prev ();
    void make_fake ();
    bool is_fake () const;

    char (*path)[12];
    uint32_t path_sz;
    light_playlist lpl;
    /*
    uint32_t pos;
    playlist_header header;
    song_header song;
    file_descriptor fd;
    */
    
private:
    void init_base ();
};

extern uint32_t memory_limit;

#endif

