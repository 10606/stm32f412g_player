#ifndef FIND_SONG_H
#define FIND_SONG_H

#include <stddef.h>
#include <stdint.h>
#include "light_playlist.h"
#include "playlist_structures.h"

struct find_song
{
    constexpr find_song () :
        find_song(light_playlist(), find_pattern{0, 0, {}, {}})
    {}
        
    constexpr find_song (light_playlist const & cur_playlist, find_pattern const & _pattern) :
        playlist(cur_playlist),
        pattern(_pattern),
        song_pf{},
        group_pf{}
    {
        pattern.song_len  = std::min(pattern.song_len, 
                                     static_cast <uint32_t> (std::extent_v <decltype(pattern.song_name)>));
        pattern.group_len = std::min(pattern.group_len, 
                                     static_cast <uint32_t> (std::extent_v <decltype(pattern.group_name)>));
        calc_prefix_function(song_pf, pattern.song_name, pattern.song_len);
        calc_prefix_function(group_pf, pattern.group_name, pattern.group_len);
    }
    
    constexpr find_song (find_song const &) = default;
    constexpr find_song (find_song &&) = default;
    constexpr find_song & operator = (find_song const &) = default;
    constexpr find_song & operator = (find_song &&) = default;
    
    constexpr void reset ()
    {
        playlist.make_fake();
    }

    constexpr find_pattern search_pattern () const
    {
        return pattern;
    }
    
    ret_code next (file_descriptor const & backup);
    
    light_playlist playlist;
    static const uint32_t not_found = 404;
    
private:
    static constexpr void calc_prefix_function (int32_t * dst, char const * src, uint32_t length) noexcept
    {
        if (length == 0)
            return;
        
        dst[0] = -1;
        int32_t j = -1;
        for (uint32_t i = 0; i != length; ++i)
        {
            while ((j != -1) && (src[j] != src[i]))
                j = dst[j];
            dst[i + 1] = ++j;
        }
    }

    static bool find_substr (int32_t const * prefix_function, char const * substr, uint32_t str_size,
                             char const * text, uint32_t text_size) noexcept;
    
    find_pattern pattern;
    int32_t song_pf[sz::song_name + 1];
    int32_t group_pf[sz::group_name + 1];
};

#endif

