#include "find_song.h"

#include "audio.h"


bool find_song::find_substr (int32_t const * prefix_function, char const * substr, uint32_t str_size,
                             char const * text, uint32_t text_size) noexcept
{
    if (str_size == 0)
        return 1;
    
    int32_t pos = 0;
    for (uint32_t i = 0; i != text_size; ++i)
    {
        while ((pos != -1) && (substr[pos] != text[i]))
            pos = prefix_function[pos];
        pos++;
        if (static_cast <uint32_t> (pos) == str_size) [[unlikely]]
            return 1;
    }
    
    return 0;
}


ret_code find_song::next (file_descriptor const & backup)
{
    for (uint32_t i = 0; i != playlist.header.cnt_songs; ++i)
    {
        if (i % 100 == 0)
            audio_ctl.audio_process();
        
        ret_code ret = playlist.next(backup);
        if (ret)
            return ret;
        
        if (find_substr(song_pf, pattern.song_name, pattern.song_len, playlist.song.song_name, sz::song_name) &&
            find_substr(group_pf, pattern.group_name, pattern.group_len, playlist.song.group_name, sz::group_name))
            return 0;
    }
    
    return not_found;
}

