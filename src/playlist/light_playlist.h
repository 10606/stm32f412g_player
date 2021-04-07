#ifndef LIGHT_PLALYLIST_H
#define LIGHT_PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include "FAT.h"
#include "playlist_structures.h"

struct light_playlist 
{
    light_playlist ();
    light_playlist (light_playlist const &);
    light_playlist & operator = (light_playlist const &);
    
    uint32_t seek (uint32_t pos);
    uint32_t next ();
    uint32_t open_file ();

    playlist_header header;
    song_header song;
    file_descriptor fd;
    uint32_t pos;
    
private:
    void copy (light_playlist const &);
    void init_base ();
};

#endif

