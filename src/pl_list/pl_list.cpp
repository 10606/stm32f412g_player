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
    current_state{0, 0, redraw_type_t::not_easy}
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
    current_state.pos = 0;
    path_len = 0;
    current_state.type = redraw_type_t::not_easy;
    root_path = (char (*)[12])malloc((len_name + 1) * sizeof(char[12]));
    if (!root_path)
        return memory_limit;
    path_len = len_name + 1;
    for (size_t i = 0; i != len_name; ++i)
        memcpy(root_path[i], dir_name[i], sizeof(char[12]));
    
 
    file_descriptor file;
    file_descriptor dir;
    uint32_t index = 0;
    uint32_t res;
    char name[12];

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
                    memmove(pl_path[index], name, sizeof(name));
                    index++;
                    cnt = index;
                }
            }
        }
    }
    cnt = index;
    
   
    for (uint32_t i = 0; i != cnt; ++i)
    {
        memcpy(root_path[path_len - 1], pl_path[i], sizeof(char[12]));
        res = open(&FAT_info, &file, root_path, path_len);
        if (res != 0)
        {
            cnt = i;
            destroy();
            return res;
        }
        
        playlist_header header;
        res = file.read_all_fixed((char *)&header, sizeof(header));
        if (res != 0)
        {
            cnt = i;
            destroy();
            return res;
        }
        memmove(pl_name[i], header.playlist_name, sizeof(playlist_header::playlist_name));
        pl_songs[i] = header.cnt_songs;
    }
    return 0;
}

void pl_list::next ()
{
    if (cnt == 0)
        return;
    
    if ((current_state.pos + 1 == cnt) || (current_state.type != redraw_type_t::nothing))
        current_state.type = redraw_type_t::not_easy;
    else if (current_state.pos < plb_border_cnt) // on top
        current_state.type = redraw_type_t::top_bottom;
    else if (current_state.pos + plb_border_cnt + 1 >= cnt) // on bottom
        current_state.type = redraw_type_t::top_bottom;
    else // on middle
        current_state.type = redraw_type_t::middle;
        
    current_state.pos = (current_state.pos + 1) % cnt;
    current_state.direction = 0;
}

void pl_list::prev ()
{
    if (cnt == 0)
        return;
    
    if ((current_state.pos == 0) || (current_state.type != redraw_type_t::nothing))
        current_state.type = redraw_type_t::not_easy;
    else if (current_state.pos <= plb_border_cnt) // on top
        current_state.type = redraw_type_t::top_bottom;
    else if (current_state.pos + plb_border_cnt >= cnt) // on bottom
        current_state.type = redraw_type_t::top_bottom;
    else // on middle
        current_state.type = redraw_type_t::middle;
        
    current_state.pos = (current_state.pos + cnt - 1) % cnt;
    current_state.direction = 1;
}

void pl_list::seek (uint32_t pos)
{
    if (cnt == 0)
        return;
    current_state.type = redraw_type_t::not_easy;
    current_state.pos = pos % cnt;
}

uint32_t pl_list::open_index (playlist_view & plv, uint32_t index, uint32_t & selected_pl) const
{
    if (cnt == 0)
        return 0;
    if (selected_pl == index)
        return 0;
    char old_path [12];
    memmove(old_path, root_path[path_len - 1], sizeof(char[12]));
    memcpy(root_path[path_len - 1], pl_path[index], sizeof(char[12]));
    uint32_t ret = plv.open_playlist(root_path, path_len);
    if (ret)
    {
        memcpy(root_path[path_len - 1], old_path, sizeof(char[12]));
        return ret;
    }
    selected_pl = index;
    return 0;
}

uint32_t pl_list::open_selected (playlist_view & plv, uint32_t & selected_pl) const
{
    return open_index(plv, current_state.pos, selected_pl);
}

bool pl_list::check_near (uint32_t pos) const
{
    if (cnt == 0)
        return 0;
    if (pos >= cnt)    
        return 0;
    
    return ::check_near
    (
        current_state.pos, 
        pos, 
        cnt, 
        plb_view_cnt, 
        plb_border_cnt
    );
}

uint32_t pl_list::print
(
    uint32_t playing_pl,
    char (* playlist_name)[sz::count_offset + sz::count + 1], 
    char * selected
) const
{
    memset(selected, 0, plb_view_cnt);
    uint32_t index = 0;
    uint32_t print_cnt = plb_view_cnt;
    
    if (cnt < plb_view_cnt)
    {
        index = 0;
        print_cnt = cnt;
        
        if (cnt != 0)
        {
            selected[current_state.pos] |= 1;
            if (playing_pl < cnt)
                selected[playing_pl] |= 2;
        }
        for (uint32_t i = cnt; i != plb_view_cnt; ++i)
        {
            memset(playlist_name[i], ' ', sizeof(playlist_name[i]));
            playlist_name[i][sizeof(playlist_name[i]) - 1] = 0;
        }
    }
    else
    {
        print_cnt = plb_view_cnt;
        if (current_state.pos <= plb_border_cnt) // on top
        {
            selected[current_state.pos] |= 1;
            index = 0;
        }
        else if (current_state.pos + plb_border_cnt >= cnt) // on bottom
        {
            selected[cnt - plb_view_cnt + current_state.pos] |= 1;
            index = cnt - plb_view_cnt;
        }
        else // on middle
        {
            selected[plb_border_cnt] |= 1;
            index = current_state.pos - plb_border_cnt;
        }

        if (check_near(playing_pl))
            selected[playing_pl - index] |= 2;
    }
    
    for (uint32_t i = 0; i != print_cnt; ++i)
    {
        uint32_t songs_cnt = (pl_songs[index + i] > 999)? 999 : pl_songs[index + i];
        sprint_mod_1000(playlist_name[i], sz::number, index + i);
        sprint(playlist_name[i] + sz::count_offset, sz::count, "%3lu", songs_cnt);
        
        memcpy(playlist_name[i] + sz::number, pl_name[index + i], sz::pl_name);
        const uint32_t sz_num_pl = sz::number + sz::pl_name;
        memset(playlist_name[i] + sz_num_pl, ' ', sz::count_offset - sz_num_pl);
        playlist_name[i][sizeof(playlist_name[i]) - 1] = 0;
    }
    return 0;
}

void pl_list::reset_display ()
{
    current_state.type = redraw_type_t::nothing;
}

redraw_type_t pl_list::redraw_type () const
{
    redraw_type_t ans = current_state;

    if (current_state.pos <= plb_border_cnt) // on top
        ans.pos = current_state.pos;
    else if (current_state.pos + plb_border_cnt >= cnt) // on bottom
        ans.pos = current_state.pos + plb_view_cnt - cnt;
    else // on middle
        ans.pos = plb_border_cnt;
    return ans;
}

