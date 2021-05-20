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
    memcpy(root_path, dir_name, len_name * sizeof(char[12]));
 
    file_descriptor file;
    file_descriptor dir;
    uint32_t index = 0;
    uint32_t res;
    
    // get paths
    res = open(&FAT_info, &dir, dir_name, len_name);
    if (res != 0)
    {
        cnt = 0;
        destroy();
        return res;
    }
    for (;;) 
    {
        if (index == max_plb_files)
            break;
        res = read_dir(&dir, &file, pl_path[index]);
        if (res != 0 || pl_path[index][0] == 0)
            break;

        if (!file.is_dir && pl_path[index][0] != '.')
            if (strncmp(pl_path[index] + 8, "PLB", 3) == 0)
                index++;
    }
    cnt = index;
   
    // get playlist names and count of songs
    for (uint32_t i = 0; i != cnt; ++i)
    {
        memcpy(root_path[path_len - 1], pl_path[i], sizeof(char[12]));
        res = open(&FAT_info, &file, root_path, path_len);
        if (res != 0)
            goto err;
        playlist_header header;
        res = file.read_all_fixed((char *)&header, sizeof(header));
        if (res != 0)
            goto err;
        memmove(pl_name[i], header.playlist_name, sizeof(playlist_header::playlist_name));
        pl_songs[i] = header.cnt_songs;
        
        continue;
    err:
        cnt = i;
        destroy();
        return res;
    }
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
        view_cnt, 
        border_cnt
    );
}

pl_list::print_info pl_list::print (uint32_t playing_pl) const
{
    print_info ans;
    uint32_t index = 0;
    uint32_t print_cnt = view_cnt;
    
    if (cnt < view_cnt)
    {
        index = 0;
        print_cnt = cnt;
        
        if (cnt != 0)
        {
            ans.selected[current_state.pos] |= 1;
            if (playing_pl < cnt)
                ans.selected[playing_pl] |= 2;
        }
        for (uint32_t i = cnt; i != view_cnt; ++i)
        {
            memset(ans.playlist_name[i], ' ', sizeof(ans.playlist_name[i]));
            ans.playlist_name[i][sizeof(ans.playlist_name[i]) - 1] = 0;
        }
    }
    else
    {
        print_cnt = view_cnt;
        if (current_state.pos <= border_cnt) // on top
        {
            ans.selected[current_state.pos] |= 1;
            index = 0;
        }
        else if (current_state.pos + border_cnt >= cnt) // on bottom
        {
            ans.selected[cnt - view_cnt + current_state.pos] |= 1;
            index = cnt - view_cnt;
        }
        else // on middle
        {
            ans.selected[border_cnt] |= 1;
            index = current_state.pos - border_cnt;
        }

        if (check_near(playing_pl))
            ans.selected[playing_pl - index] |= 2;
    }
    
    for (uint32_t i = 0; i != print_cnt; ++i)
    {
        uint32_t songs_cnt = (pl_songs[index + i] > 999)? 999 : pl_songs[index + i];
        sprint_mod_1000(ans.playlist_name[i], sz::number, index + i);
        sprint(ans.playlist_name[i] + sz::count_offset, sz::count, "%3lu", songs_cnt);
        
        memcpy(ans.playlist_name[i] + sz::number, pl_name[index + i], sz::pl_name);
        const uint32_t sz_num_pl = sz::number + sz::pl_name;
        memset(ans.playlist_name[i] + sz_num_pl, ' ', sz::count_offset - sz_num_pl);
        ans.playlist_name[i][sizeof(ans.playlist_name[i]) - 1] = 0;
    }
    
    return ans;
}

void pl_list::reset_display ()
{
    current_state.type = redraw_type_t::nothing;
}

redraw_type_t pl_list::redraw_type () const
{
    redraw_type_t ans = current_state;

    if (current_state.pos <= border_cnt) // on top
        ans.pos = current_state.pos;
    else if (current_state.pos + border_cnt >= cnt) // on bottom
        ans.pos = current_state.pos + view_cnt - cnt;
    else // on middle
        ans.pos = border_cnt;
    return ans;
}

