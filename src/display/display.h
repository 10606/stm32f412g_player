#ifndef DISPLAY_H
#define DISPLAY_H

#include "view.h"
#include "playlist.h"
#include "playlist_view.h"
#include "audio.h"
#include "playlist.h"
#include "pl_list.h"
#include "char_sizes.h"
#include <stdint.h>

namespace display
{

void cur_song (playlist * pl_p, char to_screen, uint8_t * need_redraw);
void cur_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p, char to_screen, uint8_t * need_redraw);
void cur_playlist (playlist_view * plv, playlist * pl_p, char to_screen, uint8_t * need_redraw);
void song (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_sreen, char redraw_picture, uint8_t * need_redraw);
void song_volume (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, uint8_t * need_redraw);
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

