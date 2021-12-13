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
    memcpy(old_path, root_path[path_len - 1], sizeof(filename_t));
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

