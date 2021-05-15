#ifndef PL_LIST_H
#define PL_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include "playlist_view.h"
#include "playlist_structures.h"

#define plb_border_cnt 3
#define plb_view_cnt (2 * plb_border_cnt + 1)
#define max_plb_files 25

struct pl_list
{
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

    uint32_t print 
    (
        uint32_t playing_pl,
        char (* playlist_name)[pl_name_sz + 1], 
        char (* number)[3 + 1],
        char (* count)[3 + 1], 
        char * selected
    ) const;

    void reset_display ();
    redraw_type_t redraw_type () const;
    
private:
    void destroy ();

    char (* root_path)[12];
    uint32_t path_len;
    uint32_t cnt;
    
    redraw_type_t current_state;
    
    char pl_path[max_plb_files][12];
    char pl_name[max_plb_files][pl_name_sz];
    uint32_t pl_songs[max_plb_files];
};

#endif

