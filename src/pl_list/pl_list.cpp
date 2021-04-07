#include "pl_list.h"

#include "FAT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "playlist.h"
#include "playlist_view.h"
#include "playlist_structures.h"

pl_list::pl_list () :
    root_path(nullptr),
    path_len(0),
    cnt(0),
    current_pos(0)
{}

pl_list::~pl_list ()
{
    destroy();
}

void pl_list::destroy ()
{
    free(root_path);
    root_path = nullptr;
    path_len = 0;
}

uint32_t pl_list::init (char (* dir_name)[12], size_t len_name)
{
    cnt = 0;
    current_pos = 0;
    path_len = 0;
    root_path = 0;
    root_path = (char (*)[12])malloc((len_name + 1) * 12);
    if (!root_path)
        return memory_limit;
    path_len = len_name + 1;
    for (size_t i = 0; i != len_name; ++i)
        memcpy(root_path[i], dir_name[i], 12);
    
 
    file_descriptor file;
    char name[12];
    file_descriptor dir;
    uint32_t index = 0;
    uint32_t res;

    res = open(&FAT_info, &dir, dir_name, len_name);
  
    if (res != 0)
    {
        cnt = 0;
        destroy();
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
                    memcpy(pl_path[index], name, sizeof(name));
                    index++;
                    cnt = index;
                }
            }
        }
    }
    cnt = index;
    
   
    for (uint32_t i = 0; i != cnt; ++i)
    {
        memcpy(root_path[path_len - 1], pl_path[i], 12);
        res = open(&FAT_info, &file, root_path, path_len);
        if (res != 0)
        {
            cnt = i;
            destroy();
            return res;
        }
        
        playlist_header header;
        uint32_t br;
        res = f_read_all_fixed(&file, (char *)&header, sizeof(header), &br);
        if (res != 0)
        {
            cnt = i;
            destroy();
            return res;
        }
        memcpy(pl_name[i], header.playlist_name, pl_name_sz);
        pl_songs[i] = header.cnt_songs;
    }
    return 0;
}

void pl_list::up ()
{
    if (cnt == 0)
        return;
    current_pos = (current_pos + 1) % cnt;
}

void pl_list::down ()
{
    if (cnt == 0)
        return;
    current_pos = (current_pos + cnt - 1) % cnt;
}

void pl_list::seek (uint32_t pos)
{
    if (cnt == 0)
        return;
    current_pos = pos % cnt;
}

uint32_t pl_list::open_index (playlist_view * plv, uint32_t index, uint32_t * selected_pl)
{
    if (cnt == 0)
        return 0;
    if (*selected_pl == index)
        return 0;
    char old_path [12];
    memcpy(old_path, root_path[path_len - 1], 12);
    memcpy(root_path[path_len - 1], pl_path[index], 12);
    uint32_t ret = plv->open_playlist(root_path, path_len);
    if (ret)
    {
        memcpy(root_path[path_len - 1], old_path, 12);
        return ret;
    }
    *selected_pl = index;
    return 0;
}

uint32_t pl_list::open_selected (playlist_view * plv, uint32_t * selected_pl)
{
    return open_index(plv, current_pos, selected_pl);
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

bool pl_list::check_near (uint32_t pos)
{
    if (cnt == 0)
        return 0;
    if (pos >= cnt)    
        return 0;
    
    return ::check_near
    (
        current_pos, 
        pos, 
        cnt, 
        plb_view_cnt, 
        plb_border_cnt
    );
}

uint32_t pl_list::print
(
    uint32_t playing_pl,
    char (* playlist_name)[pl_name_sz + 1], 
    char (* number)[3 + 1],
    char (* count)[3 + 1], 
    char * selected
)
{
    memset(selected, 0, plb_view_cnt);
    if (cnt < plb_view_cnt)
    {
        if (cnt != 0)
        {
            selected[current_pos] |= 1;
            if (playing_pl < cnt)
                selected[playing_pl] |= 2;
        }
        for (uint32_t i = 0; i != cnt; ++i)
        {
            fill_name(playlist_name[i], pl_name[i], pl_name_sz + 1);
            snprintf(count[i], sizeof(count[i]), "%3lu", pl_songs[i]);
            snprintf(number[i], sizeof(number[i]), "%3lu", (i % 1000));
        }
        for (uint32_t i = cnt; i != plb_view_cnt; ++i)
        {
            memset(playlist_name[i], ' ', pl_name_sz);
            playlist_name[i][pl_name_sz] = 0;
            memset(count[i], ' ', sizeof(count[i]));
            memset(number[i], ' ', sizeof(number[i]));
        }
        return 0;
    }
    
    uint32_t index;
    if (current_pos <= plb_border_cnt) // on top
    {
        selected[current_pos] |= 1;
        index = 0;
    }
    else if (current_pos + plb_border_cnt >= cnt) // on bottom
    {
        selected[cnt - plb_view_cnt + current_pos] |= 1;
        index = cnt - plb_view_cnt;
    }
    else // on middle
    {
        selected[plb_border_cnt] |= 1;
        index = current_pos - plb_border_cnt ;
    }

    if (check_near(playing_pl))
        selected[playing_pl - index] |= 2;
    
    for (uint32_t i = 0; i != plb_view_cnt; ++i)
    {
        fill_name(playlist_name[i], pl_name[i], pl_name_sz + 1);
        snprintf(count[i], sizeof(count[i]), "%3lu", pl_songs[index + i]);
        snprintf(number[i], sizeof(number[i]), "%3lu", (index + i) % 1000);
    }
    return 0;
}

