#ifndef PLALYLIST_H
#define PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"

struct playlist 
{
    playlist ();
    playlist (playlist && other);
    playlist & operator = (playlist && other);
    ~playlist ();
    
    uint32_t set_file (file_descriptor * fd, uint32_t pos_selected);
    uint32_t seek (uint32_t pos);
    uint32_t next ();
    uint32_t prev ();
    void make_fake ();
    bool is_fake ();

    file_descriptor fd;
    song_header song;
    playlist_header header;
    uint32_t path_sz;
    char (*path)[12];
    uint32_t pos;
    
private:
    uint32_t _init_playlist ();
    void move (playlist &&);
};

extern uint32_t memory_limit;

#endif

