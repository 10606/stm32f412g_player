#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "play.h"
#include "pl_list.h"
#include "FAT.h"
#include "audio.h"

extern uint32_t no_plb_files;

void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p);
void display_playlist (playlist_view * plv, playlist * pl_p);

typedef enum state_t
{
    D_PL_LIST,
    D_PLAYLIST
} state_t;

typedef struct view
{
    state_t state;
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

