#include "pl_list.h"

#include "FAT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "playlist.h"
#include "playlist_view.h"
#include "playlist_structures.h"

ret_code pl_list::init (filename_t * dir_name, size_t len_name)
{
    cnt = 0;
    current_state.pos = 0;
    path_len = 0;
    current_state.type = redraw_type_t::not_easy;
    root_path = (filename_t *)malloc((len_name + 1) * sizeof(filename_t));
    if (!root_path)
        return memory_limit;
    path_len = len_name + 1;
    memcpy(root_path, dir_name, len_name * sizeof(filename_t));
 
    file_descriptor file;
    file_descriptor dir;
    uint32_t index = 0;
    ret_code res;
    
    // get paths, names and count of songs
    res = open(&FAT_info, &dir, dir_name, len_name);
    if (res != 0)
    {
        destroy();
        return res;
    }
    for (; index < max_plb_files;) 
    {
        res = read_dir(&dir, &file, pl_path[index]);
        if (res != 0 || pl_path[index][0] == 0)
            break;

        if (!file.is_dir && 
            pl_path[index][0] != '.' &&
            strncmp(pl_path[index] + 8, "PLB", 3) == 0) // .plb
        {
            res = light_playlist::read_header(headers + index, file);
            if (res != 0)
                continue;
            index++;
        }
    }
    cnt = index;
    return 0;
}

void pl_list::next ()
{
    if (cnt == 0)
        return;
    
    if ((current_state.pos + 1 == cnt) || (current_state.type != redraw_type_t::nothing))
        current_state.type = redraw_type_t::not_easy;
    else if (current_state.pos < border_cnt) // on top
        current_state.type = redraw_type_t::top_bottom;
    else if (current_state.pos + border_cnt + 1 >= cnt) // on bottom
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
    else if (current_state.pos <= border_cnt) // on top
        current_state.type = redraw_type_t::top_bottom;
    else if (current_state.pos + border_cnt >= cnt) // on bottom
        current_state.type = redraw_type_t::top_bottom;
    else // on middle
        current_state.type = redraw_type_t::middle;
        
    current_state.pos = (current_state.pos + cnt - 1) % cnt;
    current_state.direction = 1;
}

ret_code pl_list::open_index (playlist_view & plv, uint32_t index, uint32_t & selected_pl) const
{
    if (cnt == 0)
        return 0;
    if (selected_pl == index)
        return 0;
    filename_t old_path;
    memmove(old_path, root_path[path_len - 1], sizeof(filename_t));
    memcpy(root_path[path_len - 1], pl_path[index], sizeof(filename_t));
    ret_code ret = plv.open_playlist(root_path, path_len);
    if (ret)
    {
        memcpy(root_path[path_len - 1], old_path, sizeof(filename_t));
        return ret;
    }
    selected_pl = index;
    return 0;
}

ret_code pl_list::open_selected (playlist_view & plv, uint32_t & selected_pl) const
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
        view_cnt, 
        border_cnt
    );
}

pl_list::print_info pl_list::print (uint32_t playing_pl) const
{
    print_info ans;
    uint32_t index = 0;
    uint32_t print_cnt = view_cnt;
    
    if (cnt <= view_cnt)
    {
        print_cnt = cnt;
        
        if (cnt != 0)
            ans.selected[current_state.pos] |= 1;
        for (uint32_t i = cnt; i != view_cnt; ++i)
        {
            memset(ans.playlist_name[i], ' ', sizeof(ans.playlist_name[i]));
            ans.playlist_name[i][sizeof(ans.playlist_name[i]) - 1] = 0;
        }
    }
    else
    {
        index = calc_index_set_selected <border_cnt, view_cnt> (current_state.pos, cnt, ans.selected);
    }
    
    if (check_near(playing_pl))
        ans.selected[playing_pl - index] |= 2;
    
    for (uint32_t i = 0; i != print_cnt; ++i)
    {
        uint32_t songs_cnt = (headers[index + i].cnt_songs > 999)? 999 : headers[index + i].cnt_songs;
        sprint_mod_1000(ans.playlist_name[i], sz::number, index + i);
        sprint(ans.playlist_name[i] + sz::count_offset, sz::count, "%3lu", songs_cnt);
        
        memcpy(ans.playlist_name[i] + sz::number, headers[index + i].playlist_name, sz::pl_name);
        const uint32_t sz_num_pl = sz::number + sz::pl_name;
        memset(ans.playlist_name[i] + sz_num_pl, ' ', sz::count_offset - sz_num_pl);
        ans.playlist_name[i][sizeof(ans.playlist_name[i]) - 1] = 0;
    }
    
    return ans;
}

