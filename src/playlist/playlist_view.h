#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"
#include "FAT.h"

#define playlist_border_cnt 3
#define playlist_view_cnt (2 * playlist_border_cnt + 1)

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

    void print
    (
        playlist const & playing_pl,
        char (* song_name)[sz::song_name + 1],
        char (* group_name)[sz::group_name + 1],
        char * selected,
        char (* number)[3 + 1]
    ) const;

    void reset_display ();
    redraw_type_t redraw_type () const;

    uint32_t open_playlist
    (
        char const (* const path)[12],
        uint32_t path_len
    );


    uint32_t pos_begin;
    redraw_type_t current_state;
    light_playlist lpl;
    char name_group[playlist_view_cnt][sizeof(song_header::group_name) + 1];
    char name_song[playlist_view_cnt][sizeof(song_header::song_name) + 1];
    
private:
    uint32_t fill_names ();
    void pn_common (size_t ins_pos);
    void copy_from_lpl (size_t ins_pos);
};


#endif

