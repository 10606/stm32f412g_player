#ifndef DISPLAY_SONG_H
#define DISPLAY_SONG_H

#include "playlist.h"
#include "audio.h"
#include "view_states.h"

namespace display
{

void song        (audio_ctl_t const & actl, state_song_view_t state, bool to_screen, bool redraw_screen, bool & need_redraw);
void song_volume (audio_ctl_t const & actl, state_song_view_t state, bool to_screen, bool & need_redraw);
void cur_song (playlist const & pl, bool & need_redraw);
void song_hint ();

}

#endif

