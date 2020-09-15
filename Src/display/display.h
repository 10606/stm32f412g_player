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

uint8_t AUDIO_Process(void);
void display_cur_song (playlist * pl_p);
void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p);
void display_playlist (playlist_view * plv, playlist * pl_p);
void display_song (playlist * pl, audio_ctl * actl, state_song_view_t * state);
void display_song_volume (playlist * pl, audio_ctl * actl, state_song_view_t * state);

#endif

