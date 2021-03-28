#ifndef DISPLAY_H
#define DISPLAY_H

#include "view.h"
#include "play.h"
#include "audio.h"
#include "playlist.h"
#include "pl_list.h"
#include "char_sizes.h"
#include <stdint.h>

void display_cur_song (playlist * pl_p, char to_screen, uint8_t * need_redraw);
void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p, char to_screen, uint8_t * need_redraw);
void display_playlist (playlist_view * plv, playlist * pl_p, char to_screen, uint8_t * need_redraw);
void display_song (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_sreen, char redraw_picture, uint8_t * need_redraw);
void display_song_volume (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, uint8_t * need_redraw);
void display_start_image ();
void display_error (char const * msg);
void display_song_hint ();


const uint32_t in_line_offset = 12;
const uint32_t line_offset = 24;
const uint32_t list_offset = 68;
const uint32_t song_name_offset = 7;
const uint32_t in_song_name_offset = 20;
const uint32_t time_offset = 49;

const uint32_t picture_offset = 65;
const uint32_t headband_height = 65;

const uint16_t * const song_picture_address = reinterpret_cast <uint16_t *> (0x08080000lu);
const uint16_t * const err_picture_address = reinterpret_cast <uint16_t *> (0x080c0000lu);

#endif

