#ifndef PL_LIST_H
#define PL_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include "playlist_view.h"
#include "playlist_structures.h"


struct pl_list
{
    static const uint32_t border_cnt = playlist_view::border_cnt;
    static const uint32_t view_cnt = playlist_view::view_cnt;
    static const uint32_t max_plb_files = 25;

    constexpr pl_list () :
        root_path(nullptr),
        path_len(0),
        cnt(0),
        current_state{0, 0, redraw_type_t::not_easy},
        pl_path{},
        headers{}
    {}

    constexpr void reset ()
    {
        destroy();
        current_state = {0, 0, redraw_type_t::not_easy};
    }
    
    ~pl_list ()
    {
        destroy();
    }

        
    pl_list (pl_list const &) = delete;
    pl_list & operator = (pl_list const &) = delete;
    pl_list (pl_list &&) = delete;
    pl_list & operator = (pl_list &&) = delete;
    
    ret_code init (filename_t * dir_name, size_t len_name);
    void next ();
    void prev ();
    ret_code open_index (playlist_view & plv, uint32_t index, uint32_t & selected_pl) const;
    ret_code open_selected (playlist_view & plv, uint32_t & selected_pl) const;
    bool check_near (uint32_t pos) const;

    constexpr void seek (uint32_t pos)
    {
        if (cnt == 0)
            return;
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = pos % cnt;
    }

    
    struct print_info
    {
        print_info ()
        {
            memset(selected, 0, view_cnt);
        }
    
        char playlist_name[view_cnt][sz::count_offset + sz::count + 1];
        char selected[view_cnt];
    };
    
    template <size_t count>
    print_info print (std::array <uint32_t const *, count> playlists) const;
    
    constexpr void reset_display ()
    {
        current_state.type = redraw_type_t::nothing;
    }

    constexpr redraw_type_t redraw_type () const
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

    
private:
    constexpr void destroy ()
    {
        free(root_path);
        root_path = nullptr;
        path_len = 0;
        cnt = 0;
    }

    
    filename_t * root_path;
    uint32_t path_len;
    uint32_t cnt;
    
    redraw_type_t current_state;
    
    filename_t pl_path[max_plb_files];
    playlist_header headers[max_plb_files];
};


template <size_t count>
pl_list::print_info pl_list::print (std::array <uint32_t const *, count> playlists) const
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
    
    for (size_t i = 0; i != count; ++i)
    {
        if (check_near(*playlists[i]))
            ans.selected[*playlists[i] - index] |= 2 << i;
    }
    
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
    

struct playing
{
    uint32_t playlist_index;
    playlist value;

    constexpr playing () :
        playlist_index(pl_list::max_plb_files),
        value()
    {}
        
    constexpr void reset ()
    {
        playlist_index = pl_list::max_plb_files;
        value.make_fake();
    }

    constexpr void swap (playing & other)
    {
        value.swap(other.value);
        std::swap(playlist_index, other.playlist_index);
    }
};

#endif

