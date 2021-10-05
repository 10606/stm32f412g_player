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
    
    uint32_t init (filename_t * dir_name, size_t len_name);
    void next ();
    void prev ();
    uint32_t open_index (playlist_view & plv, uint32_t index, uint32_t & selected_pl) const;
    uint32_t open_selected (playlist_view & plv, uint32_t & selected_pl) const;
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
    
    print_info print (uint32_t playing_pl) const;
    
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

#endif

