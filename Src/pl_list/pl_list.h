#ifndef PL_LIST_H
#define PL_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include "playlist_view.h"
#include "playlist_structures.h"

#define plb_border_cnt 3
#define plb_view_cnt (2 * plb_border_cnt + 1)
#define max_plb_files 25

typedef struct pl_list
{
    char (* root_path)[12];
    uint32_t path_len;
    uint32_t cnt;
    uint32_t current_pos;
    char pl_path[max_plb_files][12];
    char pl_name[max_plb_files][pl_name_sz];
    uint32_t pl_songs[max_plb_files];
} pl_list;

uint32_t init_pl_list (pl_list * pll, char (* dir_name)[12], size_t len_name);
void destroy_pl_list (pl_list * pll);
void up_pl_list (pl_list * pll);
void down_pl_list (pl_list * pll);
void seek_pl_list (pl_list * pll, uint32_t pos);
uint32_t open_selected_pl_list (pl_list * pll, playlist_view * plv, uint32_t * selected_pl);
char pl_list_check_near (pl_list * pll, uint32_t pos);

uint32_t print_pl_list 
(
    pl_list * pll, 
    uint32_t playing_pl,
    char (* restrict playlist_name)[pl_name_sz + 1], 
    char (* restrict number)[3 + 1],
    char (* restrict count)[3 + 1], 
    char * restrict selected
);

#endif

