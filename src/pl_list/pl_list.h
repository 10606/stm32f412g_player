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

    pl_list ();
    ~pl_list ();
    
    pl_list (pl_list const &) = delete;
    pl_list & operator = (pl_list const &) = delete;
    pl_list (pl_list &&) = delete;
    pl_list & operator = (pl_list &&) = delete;
    
    uint32_t init (char (* dir_name)[12], size_t len_name);
    void next ();
    void prev ();
    void seek (uint32_t pos);
    uint32_t open_index (playlist_view & plv, uint32_t index, uint32_t & selected_pl) const;
    uint32_t open_selected (playlist_view & plv, uint32_t & selected_pl) const;
    bool check_near (uint32_t pos) const;

    
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
    void reset_display ();
    redraw_type_t redraw_type () const;
    
private:
    void destroy ();

    char (* root_path)[12]; // TODO typedef char filename_type [12];
    uint32_t path_len;
    uint32_t cnt;
    
    redraw_type_t current_state;
    
    char pl_path[max_plb_files][12];
    playlist_header headers[max_plb_files];
};

#endif

