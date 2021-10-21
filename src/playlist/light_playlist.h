#ifndef LIGHT_PLALYLIST_H
#define LIGHT_PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "FAT.h"
#include "playlist_structures.h"

struct light_playlist 
{
    constexpr light_playlist () :
        header{0, {}},
        song{0, 0, {}, {}},
        fd(),
        pos(0)
    {
        init_base();
    }
    
    constexpr light_playlist (light_playlist const &) = default;
    constexpr light_playlist & operator = (light_playlist const &) = default;
    constexpr light_playlist (light_playlist &&) = default;
    constexpr light_playlist & operator = (light_playlist &&) = default;
    
    uint32_t seek (uint32_t pos);
    uint32_t seek (uint32_t pos, file_descriptor const & backup);
    uint32_t next ();
    uint32_t next (file_descriptor const & backup);
    uint32_t open_file ();

    // don't reset file_descriptor
    constexpr void init_base ()
    {
        pos = 0;
        std::fill(song.song_name, song.song_name + sizeof(song.song_name), ' ');
        std::fill(song.group_name, song.group_name + sizeof(song.group_name), ' ');
        header.cnt_songs = 0;
        std::fill(header.playlist_name, header.playlist_name + sizeof(header.playlist_name),  ' ');
    }
    
    uint32_t read_song ()
    {
        song_header old_song = song;
        uint32_t ret = fd.read_all_fixed((char *)&song, sizeof(song_header));
        if (ret)
            song = old_song;
        return ret;
    }

    static uint32_t read_header (playlist_header * header, file_descriptor & fd)
    {
        playlist_header old_header = *header;
        uint32_t ret = fd.read_all_fixed((char *)header, sizeof(playlist_header));
        if (ret)
            *header = old_header;
        return ret;
    }

    
    playlist_header header;
    song_header song;
    file_descriptor fd;
    uint32_t pos;

private:
    uint32_t next_simple ();
};

#endif

