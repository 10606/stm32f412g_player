#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"
#include "FAT.h"

#include <algorithm>

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

    consteval playlist_view () :
        pos_begin(0),
        current_state{0, 0, redraw_type_t::not_easy},
        lpl(),
        name_group{},
        name_song{}
    {}
    
    constexpr void reset ()
    {
        pos_begin = 0;
        current_state = {0, 0, redraw_type_t::not_easy};
        lpl.fd.init_fake();
        lpl.init_base();
    }
    
    uint32_t init (); //set fd and read playlist
    uint32_t next ();
    uint32_t prev ();
    uint32_t seek (uint32_t pos);
    uint32_t play (playlist & pl) const;
    uint32_t to_playing_playlist (light_playlist const & pl);
    bool is_fake () const;
    bool check_near (playlist const & playing_pl) const;
    uint32_t open_playlist (filename_t const * path, uint32_t path_len);

    constexpr bool compare (light_playlist const & b) const
    {
        return lpl.fd == b.fd;
    }

    constexpr uint32_t get_pos () const
    {
        return current_state.pos;
    }

    
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
    light_playlist lpl_with_wrong_pos () const;

private:
    uint32_t fill_names (file_descriptor const & backup);
    
    constexpr void pn_common (size_t ins_pos)
    {
        pos_begin %= lpl.header.cnt_songs;
        pos_begin %= view_cnt;
        
        ins_pos %= lpl.header.cnt_songs;
        ins_pos %= view_cnt;
        
        copy_from_lpl(ins_pos);
    }
    
    constexpr void copy_from_lpl (size_t ins_pos)
    {
        std::copy(lpl.song.group_name, lpl.song.group_name + sizeof(song_header::group_name), name_group[ins_pos]);
        name_group[ins_pos][sizeof(song_header::group_name)] = 0;
        std::copy(lpl.song.song_name, lpl.song.song_name + sizeof(song_header::song_name), name_song[ins_pos]);
        name_song[ins_pos][sizeof(song_header::song_name)] = 0;
    }

    constexpr void next_prev_for_short (uint32_t diff)
    {
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = (current_state.pos + diff) % lpl.header.cnt_songs;
    }

    uint32_t jump_over (uint32_t seek_pos, uint32_t new_pos);

    uint32_t pos_begin;
    redraw_type_t current_state;
    light_playlist lpl;
    char name_group[view_cnt][sizeof(song_header::group_name) + 1];
    char name_song[view_cnt][sizeof(song_header::song_name) + 1];
};

#endif

