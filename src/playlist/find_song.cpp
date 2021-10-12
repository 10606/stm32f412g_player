#include "find_song.h"

#include "audio.h"


bool find_song::find_substr (uint32_t const * prefix_function, char const * substr, uint32_t str_size,
                              char const * text, uint32_t text_size) noexcept
{
    if (str_size == 0)
        return 1;
    
    uint32_t pos = 0;
    for (uint32_t i = 0; i != text_size; ++i)
    {
        while ((pos != 0) && (substr[pos] != text[i]))
            pos = prefix_function[pos - 1];
        pos += (text[i] == substr[pos]);
        if (pos == str_size) [[unlikely]]
            return 1;
    }
    
    return 0;
}


uint32_t find_song::next ()
{
    for (uint32_t i = 0; i != playlist.header.cnt_songs; ++i)
    {
        if (i % 100 == 0)
            audio_ctl.audio_process();
        
        uint32_t ret = playlist.next();
        if (ret)
            return ret;
        
        if (find_substr(song_pf, pattern.song_name, pattern.song_len, playlist.song.song_name, sz::song_name) &&
            find_substr(group_pf, pattern.group_name, pattern.group_len, playlist.song.group_name, sz::group_name))
            return 0;
    }
    
    return not_found;
}

