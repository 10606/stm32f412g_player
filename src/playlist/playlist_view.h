#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"
#include "FAT.h"

#define playlist_border_cnt 3
#define playlist_view_cnt (2 * playlist_border_cnt + 1)

extern FAT_info_t FAT_info;

struct playlist_view
{
    playlist_view () :
        pos_begin(0),
        pos_selected(0)
    {}
    
    uint32_t init (); //set fd and read playlist
    uint32_t down ();
    uint32_t up ();
    uint32_t seek (uint32_t pos);
    uint32_t play (playlist & pl) const;
    uint32_t to_playing_playlist (playlist const & pl);
    bool is_fake () const;
    bool check_near (playlist const & playing_pl) const;
    bool compare (playlist const & b) const;

    void print
    (
        playlist const & playing_pl,
        char (* song_name)[song_name_sz + 1],
        char (* group_name)[group_name_sz + 1],
        char * selected,
        char (* number)[3 + 1]
    ) const;

    uint32_t open_playlist
    (
        char const (* const path)[12],
        uint32_t path_len
    );


    uint32_t pos_begin;
    uint32_t pos_selected;
    light_playlist lpl;
    char name_group[playlist_view_cnt][group_name_sz + 1];
    char name_song[playlist_view_cnt][song_name_sz + 1];
    
private:
    uint32_t fill_names ();
    void pn_common (size_t ins_pos);
};


#endif

