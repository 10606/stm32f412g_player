#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pl_list.h"
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

    res = open(&dir, dir_name, len_name);
  
    if (res != 0)
    {
        //BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"No directory Media...", 0);
        pll->cnt = 0;
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
        res = open(&file, pll->root_path, pll->path_len);
        if (res != 0)
        {
            pll->cnt = i;
            return res;
        }
        
        playlist_header header;
        uint32_t tread = 0;
        uint32_t bread;
        while (tread != sizeof(header))
        {
            res = f_read(&file, (char *)&header + tread, sizeof(header) - tread, &bread);
            if (res != 0)
            {
                pll->cnt = i;
                return res;
            }
            tread += bread;
        }
        memcpy(pll->pl_name[i], header.playlist_name, pl_name_sz);
        pll->pl_songs[i] = header.cnt_songs;
    }
    return 0;
}

void up_pl_list (pl_list * pll)
{
    pll->current_pos = (pll->current_pos + 1) % pll->cnt;
}

void down_pl_list (pl_list * pll)
{
    pll->current_pos = (pll->current_pos + pll->cnt - 1) % pll->cnt;
}

void seek_pl_list (pl_list * pll, uint32_t pos)
{
    pll->current_pos = pos % pll->cnt;
}

uint32_t open_selected_pl_list (pl_list * pll, playlist_view * plv, uint32_t * selected_pl)
{
    if (*selected_pl == pll->current_pos)
        return 0;
    memcpy(pll->root_path[pll->path_len - 1], pll->pl_path[pll->current_pos], 12);
    uint32_t ret = open_playlist(plv, pll->root_path, pll->path_len);
    if (ret)
        return ret;
    *selected_pl = pll->current_pos;
    return 0;
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

uint32_t print_pl_list 
(
    pl_list * pll, 
    uint32_t playing_pl,
    char (* playlist_name)[pl_name_sz + 1], 
    char (* number)[3 + 1],
    char (* count)[3 + 1], 
    char * selected
)
{
    memset(selected, 0, view_plb_cnt);
    if (pll->cnt < view_plb_cnt)
    {
        selected[pll->current_pos] |= 1;
        if (playing_pl < pll->cnt)
            selected[playing_pl] |= 2;
        for (uint32_t i = 0; i != pll->cnt; ++i)
        {
            fill_name(playlist_name[i], pll->pl_name[i], pl_name_sz + 1);
            snprintf(count[i], sizeof(count[i]), "%3lu", pll->pl_songs[i]);
            snprintf(number[i], sizeof(number[i]), "%3lu", (i % 1000));
        }
        for (uint32_t i = pll->cnt; i != view_plb_cnt; ++i)
        {
            memset(playlist_name[i], ' ', pl_name_sz);
            playlist_name[i][pl_name_sz] = 0;
            memset(count[i], ' ', sizeof(count[i]));
            memset(number[i], ' ', sizeof(number[i]));
        }
        return 0;
    }
    
    uint32_t index;
    if (pll->current_pos <= border_plb_cnt)
    {
        selected[pll->current_pos] |= 1;
        if (playing_pl < view_plb_cnt)
            selected[playing_pl] |= 2;
        index = 0;
    }
    else if (pll->current_pos >= pll->cnt - border_plb_cnt)
    {
        selected[pll->cnt - view_plb_cnt + pll->current_pos] |= 1;
        if (playing_pl >= pll->cnt - view_plb_cnt)
            selected[pll->cnt - view_plb_cnt + playing_pl] |= 2;
        index = pll->cnt - view_plb_cnt;
    }
    else
    {
        selected[border_plb_cnt] |= 1;
        if ((playing_pl >= pll->current_pos - border_plb_cnt) &&
            (playing_pl <= pll->current_pos + border_plb_cnt))
        {
            selected[playing_pl + border_plb_cnt - pll->current_pos] |= 2;
        }
        index = pll->current_pos - border_plb_cnt ;
    }
    for (uint32_t i = 0; i != view_plb_cnt; ++i)
    {
        fill_name(playlist_name[i], pll->pl_name[i], pl_name_sz + 1);
        snprintf(count[i], sizeof(count[i]), "%3lu", pll->pl_songs[index + i]);
        snprintf(number[i], sizeof(number[i]), "%3lu", index + i);
    }
    return 0;
}

