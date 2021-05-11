#ifndef DISPLAY_H
#define DISPLAY_H

#include "view_states.h"
#include "playlist.h"
#include "playlist_view.h"
#include "audio.h"
#include "playlist.h"
#include "pl_list.h"
#include <stdint.h>

namespace display
{

void cur_song (playlist const & pl, bool & need_redraw);

void cur_pl_list  (pl_list & pll,            uint32_t playing_pl,     bool to_screen, bool redraw_screen,  bool & need_redraw);
void cur_playlist (playlist_view & plv,      playlist const & pl,     bool to_screen, bool redraw_screen,  bool & need_redraw);
void song         (audio_ctl_t const & actl, state_song_view_t state, bool to_screen, bool redraw_picture, bool & need_redraw);
void song_volume  (audio_ctl_t const & actl, state_song_view_t state, bool to_screen, bool & need_redraw);

void start_image ();
void error (char const * msg);
void song_hint ();


struct offsets
{
    static const uint32_t in_line = 12;
    static const uint32_t line = 24;
    static const uint32_t list = 68;
    static const uint32_t song_name = 7;
    static const uint32_t in_song_name = 20;
    static const uint32_t time = 49;

    static const uint32_t picture = 65;
    static const uint32_t headband = 65;
};

}

#endif

