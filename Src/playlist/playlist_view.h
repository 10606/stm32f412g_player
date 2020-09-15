#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"

#define view_border 3
#define view_cnt (2 * view_border + 1)

typedef struct playlist_view
{
    light_playlist lpl;
    uint32_t pos_begin;
    uint32_t pos_selected;
    char name_group[view_cnt][group_name_sz + 1];
    char name_song[view_cnt][song_name_sz + 1];
} playlist_view;

void init_playlist_view (playlist_view * plv, file_descriptor * fd); //set fd and read playlist
void down (playlist_view * plv);
void up (playlist_view * plv);
void seek_playlist_view (playlist_view * plv, uint32_t pos);
void play (playlist_view * plv, playlist * pl);
char compare (light_playlist * a, playlist * b);

char check_near (playlist_view * plv, playlist * playing_pl);

void print_playlist_view
(
    playlist_view * plv,
    playlist * playing_pl,
    char (* song_name)[song_name_sz + 1],
    char (* group_name)[group_name_sz + 1],
    char * selected,
    char (* number)[3 + 1]
);

void bind_playlist_view (playlist_view * plv, file_descriptor * fd); //just set file_descriptor 
uint32_t open_playlist
(
    playlist_view * plv,
    char const (* const path)[12],
    uint32_t path_len
);

#endif

