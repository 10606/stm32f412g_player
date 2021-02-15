#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "play.h"
#include "pl_list.h"
#include "FAT.h"
#include "audio.h"

typedef enum state_song_view_t
{
    S_VOLUME = 0,
    S_SEEK = 1,
    S_NEXT_PREV = 2
} state_song_view_t;
#define state_song_view_cnt 3

typedef enum state_t
{
    D_PL_LIST = 0,
    D_PLAYLIST = 1,
    D_SONG = 2
} state_t; // WARNING on change: process_view_left (view * vv, uint8_t * need_redraw)

typedef struct view
{
    state_t state;
    state_t old_state;
    state_song_view_t state_song_view;
    uint32_t playing_playlist;
    uint32_t selected_playlist;

    file_descriptor fd_plv;
    playlist_view plv;
    
    file_descriptor fd_pl;
    playlist pl;
    
    pl_list pll;

    audio_ctl_t * audio_ctl;
} view;

uint32_t init_view (view * vv, char (* path)[12], uint32_t len, audio_ctl_t * audio_ctl);
uint32_t destroy_view (view * vv);
uint32_t process_view (view * vv, uint8_t * need_redraw);
void display_view (view * vv, uint8_t * need_redraw);

uint32_t process_view_up (view * vv, uint8_t * need_redraw);
uint32_t process_view_down (view * vv, uint8_t * need_redraw);
uint32_t process_view_up_down (view * vv, uint8_t * need_redraw, uint8_t direction /* 0 - down, 1 - up */);
uint32_t process_view_left (view * vv, uint8_t * need_redraw);
uint32_t process_view_right (view * vv, uint8_t * need_redraw);
uint32_t process_view_center (view * vv, uint8_t * need_redraw);


uint32_t process_view_play_pause (view * vv, uint8_t * need_redraw);
uint32_t process_view_inc_volume (view * vv, uint8_t * need_redraw);
uint32_t process_view_seek_forward (view * vv, uint8_t * need_redraw);
uint32_t process_view_prev_song (view * vv, uint8_t * need_redraw);
uint32_t process_toggle_repeat (view * vv, uint8_t * need_redraw);

uint32_t process_view_dec_volume (view * vv, uint8_t * need_redraw);
uint32_t process_view_seek_backward (view * vv, uint8_t * need_redraw);
uint32_t process_view_next_song (view * vv, uint8_t * need_redraw);

uint32_t open_song (view * vv);
uint32_t open_song_not_found (view * vv, char direction); // direction == 1 - reverse

void fake_song_and_playlist (view * vv);

#endif

