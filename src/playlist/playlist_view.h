#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"
#include "FAT.h"


extern FAT_info_t FAT_info;

struct redraw_type_t 
{
    enum type_t
    {
        nothing,
        not_easy,
        top_bottom,
        middle
    };
    
    uint32_t pos;
    bool direction; // 0 - up
    type_t type;
};

struct playlist_view
{
    static const uint32_t border_cnt = 3;
    static const uint32_t view_cnt = (2 * border_cnt + 1);

    playlist_view () :
        pos_begin(0),
        current_state{0, 0, redraw_type_t::not_easy}
    {}
    
    uint32_t init (); //set fd and read playlist
    uint32_t next ();
    uint32_t prev ();
    uint32_t seek (uint32_t pos);
    uint32_t play (playlist & pl) const;
    uint32_t to_playing_playlist (playlist const & pl);
    bool is_fake () const;
    bool check_near (playlist const & playing_pl) const;
    bool compare (playlist const & b) const;
    uint32_t open_playlist (char const (* const path)[12], uint32_t path_len);

    
    struct print_info
    {
        print_info ()
        {
            memset(selected, 0, view_cnt);
        }
    
        char song_name[view_cnt][sz::number + sz::song_name + 1];
        char group_name[view_cnt][sz::number + sz::group_name + 1];
        char selected[view_cnt];
    };
    
    print_info print (playlist const & playing_pl_) const;
    void reset_display ();
    redraw_type_t redraw_type () const;

private:
    uint32_t fill_names ();
    void pn_common (size_t ins_pos);
    void copy_from_lpl (size_t ins_pos);
    void next_prev_for_short (uint32_t diff, uint32_t border);
    uint32_t jump_over (uint32_t seek_pos, uint32_t new_pos);

    uint32_t pos_begin;
    redraw_type_t current_state;
    light_playlist lpl;
    char name_group[view_cnt][sizeof(song_header::group_name) + 1];
    char name_song[view_cnt][sizeof(song_header::song_name) + 1];
};

#endif

