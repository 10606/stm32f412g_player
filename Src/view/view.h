#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "play.h"
#include "pl_list.h"
#include "FAT.h"
#include "audio.h"

extern uint32_t no_plb_files;

typedef enum state_song_view_t
{
    S_VOLUME = 0,
    S_SEEK = 1,
    S_NEXT_PREV = 2
} state_song_view_t;
#define state_song_view_cnt 3

typedef enum state_t
{
    D_PL_LIST,
    D_PLAYLIST,
    D_SONG
} state_t;

typedef struct view
{
    state_t state;
    state_song_view_t state_song_view;
    uint32_t playing_playlist;
    uint32_t selected_playlist;

    file_descriptor fd_plv;
    playlist_view plv;
    
    file_descriptor fd_pl;
    playlist pl;
    
    pl_list pll;

    audio_ctl * buffer_ctl;
} view;

uint32_t init_view (view * vv, char (* path)[12], uint32_t len, audio_ctl * buffer_ctl);
uint32_t destroy_view (view * vv);
uint32_t process_view (view * vv, uint8_t * need_redraw);
void display_view (view * vv);

#endif

