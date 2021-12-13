#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "playlist.h"
#include "light_playlist.h"
#include "playlist_structures.h"
#include "FAT.h"

#include <algorithm>
#include <array>

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
        lpl.make_fake();
    }
    
    ret_code init (); //set fd and read playlist
    ret_code next ();
    ret_code prev ();
    ret_code seek (uint32_t pos);
    ret_code play (playlist & pl, playlist const & backup) const;
    ret_code to_playing_playlist (light_playlist const & pl);
    bool is_fake () const;
    bool check_near (playlist const & playing_pl) const;
    ret_code open_playlist (filename_t const * path, uint32_t path_len);

    constexpr bool operator == (playlist const & other) const
    {
        return (lpl.fd == other.lpl.fd) && (current_state.pos == other.lpl.pos);
    }

    constexpr bool operator != (playlist const & other) const
    {
        return !operator == (other);
    }
    
    constexpr bool compare (light_playlist const & other) const
    {
        return lpl.fd == other.fd;
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
    
    template <size_t count>
    print_info print (std::array <playlist const *, count> plalists) const;
    
    void reset_display ();
    redraw_type_t redraw_type () const;
    light_playlist lpl_with_wrong_pos () const;

private:
    ret_code fill_names (file_descriptor const & backup);
    
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

    ret_code jump_over (uint32_t seek_pos, uint32_t new_pos);

    uint32_t pos_begin;
    redraw_type_t current_state;
    light_playlist lpl;
    char name_group[view_cnt][sizeof(song_header::group_name) + 1];
    char name_song[view_cnt][sizeof(song_header::song_name) + 1];
};


template <size_t count>
playlist_view::print_info playlist_view::print (std::array <playlist const *, count> playlists) const
{
    print_info ans;
    uint32_t index = 0;
    uint32_t print_cnt = view_cnt;
    
    if (lpl.header.cnt_songs <= view_cnt)
    {
        print_cnt = lpl.header.cnt_songs;
        
        if (lpl.header.cnt_songs != 0)
            ans.selected[current_state.pos] |= 1;
        for (size_t i = lpl.header.cnt_songs; i != view_cnt; ++i)
        {
            memset(ans.song_name[i], ' ', sizeof(ans.song_name[i]));
            memset(ans.group_name[i], ' ', sizeof(ans.group_name[i]));
            ans.song_name[i][sizeof(ans.song_name[i]) - 1] = 0;
            ans.group_name[i][sizeof(ans.group_name[i]) - 1] = 0;
        }
    }
    else
    {
        index = calc_index_set_selected <border_cnt, view_cnt> (current_state.pos, lpl.header.cnt_songs, ans.selected);
    }
    
    for (size_t i = 0; i != count; ++i)
    {
        if (check_near(*playlists[i]))
            ans.selected[playlists[i]->lpl.pos - index] |= 2 << i;
    }

    for (size_t i = 0; i != print_cnt; ++i)
    {
        uint32_t name_id = (i + pos_begin) % view_cnt;
        memcpy(ans.song_name[i] + sz::number, name_song[name_id], sz::song_name + 1);
        memcpy(ans.group_name[i] + sz::number, name_group[name_id], sz::group_name + 1);
        sprint_mod_1000(ans.group_name[i], sz::number, index + i);
        memset(ans.song_name[i], ' ', sz::number);
    }
    
    return ans;
}
    
#endif

