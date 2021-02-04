#ifndef DISPLAY_H
#define DISPLAY_H

#include "view.h"
#include "play.h"
#include "audio.h"
#include "playlist.h"
#include "pl_list.h"
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include <stdint.h>

void display_cur_song (playlist * pl_p, char to_screen, uint8_t * need_redraw);
void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p, char to_screen, uint8_t * need_redraw);
void display_playlist (playlist_view * plv, playlist * pl_p, int state, char to_screen, uint8_t * need_redraw);
void display_song (playlist * pl, audio_ctl_t * actl, state_song_view_t * state, char to_sreen, uint8_t * need_redraw);
void display_song_volume (playlist * pl, audio_ctl_t * actl, state_song_view_t * state, char to_screen, uint8_t * need_redraw);
void display_start_image ();
void display_error (char const * msg);
void display_song_hint ();


#define volume_width 10

#define in_line_offset 11
#define line_offset 23
#define list_offset 75
#define name_offset 5
#define count_offset 30
#define name_limit 11

#define picture_offset 75
#define headband_height  72
#define song_picture_address (void *)0x08080000
#define err_picture_address (void *)0x080c0000
#endif

