#ifndef DISPLAY_SONG_H
#define DISPLAY_SONG_H

#include "playlist.h"
#include "audio.h"
#include "view_states.h"

namespace display
{

inline char print_c_state (state_song_view_t state) noexcept
{
    switch (state)
    {
    case state_song_view_t::volume:
        return 'v';
    case state_song_view_t::seek:
        return 's';
    case state_song_view_t::next_prev:
        return 'n';
    default:
        return ' ';
    }
    
}

inline char print_p_state (pause_status_t pause_status) noexcept
{
    switch (pause_status)
    {
    case pause_status_t::play: [[likely]]
        return ' ';
    case pause_status_t::pause:
        return 7;
    case pause_status_t::soft_pause:
        return 6;
    default:
        return ' ';
    }
}

inline char print_r_state (bool repeat_mode) noexcept
{
    return repeat_mode? 'r' : ' ';
}

void song        (audio_ctl_t const & actl, state_song_view_t state, state_t cur_state, state_t old_state);
void song_volume (audio_ctl_t const & actl, state_song_view_t state, bool to_screen);
void cur_song    (audio_ctl_t const & actl, playlist const & pl);
void song_hint ();

}

#endif

