#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"

#define playlist_border_cnt 3
#define playlist_view_cnt (2 * playlist_border_cnt + 1)

typedef struct playlist_view
{
    light_playlist lpl;
    uint32_t pos_begin;
    uint32_t pos_selected;
    char name_group[playlist_view_cnt][group_name_sz + 1];
    char name_song[playlist_view_cnt][song_name_sz + 1];
} playlist_view;

uint32_t init_playlist_view (playlist_view * plv, file_descriptor * fd); //set fd and read playlist
uint32_t down (playlist_view * plv);
uint32_t up (playlist_view * plv);
uint32_t seek_playlist_view (playlist_view * plv, uint32_t pos);
uint32_t play (playlist_view * plv, playlist * pl);

char playlist_compare (light_playlist * a, playlist * b);
char playlist_check_near (playlist_view * plv, playlist * playing_pl);
char is_fake_playlist_view (playlist_view * plv);

void print_playlist_view
(
    playlist_view * plv,
    playlist * playing_pl,
    char (* restrict song_name)[song_name_sz + 1],
    char (* restrict group_name)[group_name_sz + 1],
    char * restrict selected,
    char (* restrict number)[3 + 1]
);

void bind_playlist_view (playlist_view * plv, file_descriptor * fd); //just set file_descriptor 
uint32_t open_playlist
(
    playlist_view * plv,
    char const (* const path)[12],
    uint32_t path_len
);

#endif
