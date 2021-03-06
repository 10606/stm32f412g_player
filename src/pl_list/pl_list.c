#include "pl_list.h"

#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlist.h"
#include "playlist_view.h"
#include "playlist_structures.h"
#include "FAT.h"

void destroy_pl_list (pl_list * pll)
{
    if (pll->root_path)
        free(pll->root_path);
    pll->root_path = 0;
    pll->path_len = 0;
}

uint32_t init_pl_list (pl_list * pll, char (* dir_name)[12], size_t len_name)
{
    pll->cnt = 0;
    pll->current_pos = 0;
    pll->path_len = 0;
    pll->root_path = 0;
    pll->root_path = (char (*)[12])malloc((len_name + 1) * 12);
    if (!pll->root_path)
        return memory_limit;
    pll->path_len = len_name + 1;
    for (size_t i = 0; i != len_name; ++i)
        memcpy(pll->root_path[i], dir_name[i], 12);
    
 
    file_descriptor file;
    char name[12];
    file_descriptor dir;
    uint32_t index = 0;
    uint32_t res;

    res = open(&FAT_info, &dir, dir_name, len_name);
  
    if (res != 0)
    {
        pll->cnt = 0;
        destroy_pl_list(pll);
        return res;
    }
    for (;;)
    {
        res = read_dir(&dir, &file, name);
        if ((res != 0) || (name[0] == 0))
            break;
        if (name[0] == '.')
            continue;

        if (!file.is_dir)
        {
            if (index < max_plb_files)
            {
                if ((name[8]  == 'P') && 
                    (name[9]  == 'L') && 
                    (name[10] == 'B'))
                {
                    memcpy(pll->pl_path[index], name, sizeof(name));
                    index++;
                    pll->cnt = index;
                }
            }
        }
    }
    pll->cnt = index;
    
    
    
   
    for (uint32_t i = 0; i != pll->cnt; ++i)
    {
        memcpy(pll->root_path[pll->path_len - 1], pll->pl_path[i], 12);
        res = open(&FAT_info, &file, pll->root_path, pll->path_len);
        if (res != 0)
        {
            pll->cnt = i;
            destroy_pl_list(pll);
            return res;
        }
        
        playlist_header header;
        uint32_t br;
        res = f_read_all_fixed(&file, (char *)&header, sizeof(header), &br);
        if (res != 0)
        {
            pll->cnt = i;
            destroy_pl_list(pll);
            return res;
        }
        memcpy(pll->pl_name[i], header.playlist_name, pl_name_sz);
        pll->pl_songs[i] = header.cnt_songs;
    }
    return 0;
}

void up_pl_list (pl_list * pll)
{
    if (pll->cnt == 0)
        return;
    pll->current_pos = (pll->current_pos + 1) % pll->cnt;
}

void down_pl_list (pl_list * pll)
{
    if (pll->cnt == 0)
        return;
    pll->current_pos = (pll->current_pos + pll->cnt - 1) % pll->cnt;
}

void seek_pl_list (pl_list * pll, uint32_t pos)
{
    if (pll->cnt == 0)
        return;
    pll->current_pos = pos % pll->cnt;
}

uint32_t open_index_pl_list (pl_list * pll, playlist_view * plv, uint32_t index, uint32_t * selected_pl)
{
    if (pll->cnt == 0)
        return 0;
    if (*selected_pl == index)
        return 0;
    char old_path [12];
    memcpy(old_path, pll->root_path[pll->path_len - 1], 12);
    memcpy(pll->root_path[pll->path_len - 1], pll->pl_path[index], 12);
    uint32_t ret = open_playlist(plv, pll->root_path, pll->path_len);
    if (ret)
    {
        memcpy(pll->root_path[pll->path_len - 1], old_path, 12);
        return ret;
    }
    *selected_pl = index;
    return 0;
}

uint32_t open_selected_pl_list (pl_list * pll, playlist_view * plv, uint32_t * selected_pl)
{
    return open_index_pl_list(pll, plv, pll->current_pos, selected_pl);
}

void fill_name (char * dst, char * src, uint32_t size)
{
    memset(dst, ' ', size - 1);
    memcpy(dst, src, size - 1);
    char flag = 0;
    for (uint32_t j = 0; j != size; ++j)
        if ((dst[j] == 0) || flag)
        {
            dst[j] = ' ';
            flag = 1;
        }
    dst[size - 1] = 0;
}

char pl_list_check_near (pl_list * pll, uint32_t pos)
{
    if (pll->cnt == 0)
        return 0;
    if (pos >= pll->cnt)    
        return 0;
    
    return check_near
    (
        pll->current_pos, 
        pos, 
        pll->cnt, 
        plb_view_cnt, 
        plb_border_cnt
    );
}

uint32_t print_pl_list 
(
    pl_list * pll, 
    uint32_t playing_pl,
    char (* restrict playlist_name)[pl_name_sz + 1], 
    char (* restrict number)[3 + 1],
    char (* restrict count)[3 + 1], 
    char * restrict selected
)
{
    memset(selected, 0, plb_view_cnt);
    if (pll->cnt < plb_view_cnt)
    {
        if (pll->cnt != 0)
        {
            selected[pll->current_pos] |= 1;
            if (playing_pl < pll->cnt)
                selected[playing_pl] |= 2;
        }
        for (uint32_t i = 0; i != pll->cnt; ++i)
        {
            fill_name(playlist_name[i], pll->pl_name[i], pl_name_sz + 1);
            snprintf(count[i], sizeof(count[i]), "%3lu", pll->pl_songs[i]);
            snprintf(number[i], sizeof(number[i]), "%3lu", (i % 1000));
        }
        for (uint32_t i = pll->cnt; i != plb_view_cnt; ++i)
        {
            memset(playlist_name[i], ' ', pl_name_sz);
            playlist_name[i][pl_name_sz] = 0;
            memset(count[i], ' ', sizeof(count[i]));
            memset(number[i], ' ', sizeof(number[i]));
        }
        return 0;
    }
    
    uint32_t index;
    if (pll->current_pos <= plb_border_cnt) // on top
    {
        selected[pll->current_pos] |= 1;
        index = 0;
    }
    else if (pll->current_pos + plb_border_cnt >= pll->cnt) // on bottom
    {
        selected[pll->cnt - plb_view_cnt + pll->current_pos] |= 1;
        index = pll->cnt - plb_view_cnt;
    }
    else // on middle
    {
        selected[plb_border_cnt] |= 1;
        index = pll->current_pos - plb_border_cnt ;
    }

    if (pl_list_check_near(pll, playing_pl))
        selected[playing_pl - index] |= 2;
    
    for (uint32_t i = 0; i != plb_view_cnt; ++i)
    {
        fill_name(playlist_name[i], pll->pl_name[i], pl_name_sz + 1);
        snprintf(count[i], sizeof(count[i]), "%3lu", pll->pl_songs[index + i]);
        snprintf(number[i], sizeof(number[i]), "%3lu", (index + i) % 1000);
    }
    return 0;
}

