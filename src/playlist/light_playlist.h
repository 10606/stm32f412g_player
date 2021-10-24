#ifndef LIGHT_PLALYLIST_H
#define LIGHT_PLALYLIST_H

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "FAT.h"
#include "playlist_structures.h"
#include "util.h"

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
    
    ret_code seek (uint32_t pos);
    ret_code seek (uint32_t pos, file_descriptor const & backup);
    ret_code next ();
    ret_code next (file_descriptor const & backup);
    ret_code open_file ();

    // don't reset file_descriptor
    constexpr void init_base ()
    {
        pos = 0;
        std::fill(song.song_name, song.song_name + sizeof(song.song_name), ' ');
        std::fill(song.group_name, song.group_name + sizeof(song.group_name), ' ');
        header.cnt_songs = 0;
        std::fill(header.playlist_name, header.playlist_name + sizeof(header.playlist_name),  ' ');
    }
    
    ret_code read_song ()
    {
        song_header old_song = song;
        ret_code ret = fd.read_all_fixed((char *)&song, sizeof(song_header));
        if (ret)
            song = old_song;
        return ret;
    }

    static ret_code read_header (playlist_header * header, file_descriptor & fd)
    {
        playlist_header old_header = *header;
        ret_code ret = fd.read_all_fixed((char *)header, sizeof(playlist_header));
        if (ret)
            *header = old_header;
        return ret;
    }

    
    playlist_header header;
    song_header song;
    file_descriptor fd;
    uint32_t pos;

private:
    ret_code next_simple ();
};

#endif

