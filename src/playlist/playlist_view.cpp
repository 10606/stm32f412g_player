#include "playlist_view.h"

#include "util.h"
#include <stdio.h>
#include <type_traits>

void playlist_view::copy_from_lpl (size_t ins_pos)
{
    memcpy(name_group[ins_pos], lpl.song.group_name, sizeof(song_header::group_name));
    name_group[ins_pos][sizeof(song_header::group_name)] = 0;
    memcpy(name_song[ins_pos], lpl.song.song_name, sizeof(song_header::song_name));
    name_song[ins_pos][sizeof(song_header::song_name)] = 0;
}

uint32_t playlist_view::fill_names ()
{
    if (lpl.header.cnt_songs == 0)
        return 0;

    decltype(name_group) name_group_backup;
    decltype(name_song)  name_song_backup;
    memcpy(name_group_backup, name_group, sizeof(name_group));
    memcpy(name_song_backup, name_song, sizeof(name_song));
    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        copy_from_lpl(i);
        if (i + 1 == lpl.header.cnt_songs)
            break;
        uint32_t ret = lpl.next(); 
        if (ret)
        {
            memcpy(name_group, name_group_backup, sizeof(name_group));
            memcpy(name_song, name_song_backup, sizeof(name_song));
            return ret;
        }
    }
    return 0;
}

uint32_t playlist_view::init ()
{
    uint32_t ret;
    light_playlist old_lpl(lpl);
    ret = lpl.open_file();
    if (ret)
        return ret;
    ret = fill_names();
    if (ret)
    {
        lpl = old_lpl;
        return ret;
    }
    pos_begin = 0;
    current_state.pos = 0;
    current_state.type = redraw_type_t::not_easy;
    return 0;
}

void playlist_view::pn_common (size_t ins_pos)
{
    pos_begin %= lpl.header.cnt_songs;
    pos_begin %= playlist_view_cnt;
    
    ins_pos %= lpl.header.cnt_songs;
    ins_pos %= playlist_view_cnt;
    
    copy_from_lpl(ins_pos);
}

uint32_t playlist_view::seek (uint32_t pos)
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    pos = pos % lpl.header.cnt_songs;

    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = pos;
        return 0;
    }
    
    uint32_t seek_pos;
    if (pos < playlist_border_cnt) // on top
        seek_pos = 0;
    else if (pos + playlist_border_cnt >= lpl.header.cnt_songs) // on bottom
        seek_pos = lpl.header.cnt_songs - playlist_view_cnt;
    else // on middle
        seek_pos = pos - playlist_border_cnt;

    light_playlist old_lpl(lpl);
    uint32_t ret;
    if ((ret = lpl.seek(seek_pos)))
        return ret;
    if ((ret = fill_names()))
    {
        lpl = old_lpl;
        return ret;
    }
    pos_begin = 0;
    current_state.pos = pos;
    current_state.type = redraw_type_t::not_easy;
    
    return 0;
}

uint32_t playlist_view::next ()
{
    current_state.direction = 0;
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        if (current_state.type == redraw_type_t::nothing)
        {
            if ((current_state.pos + 1) == lpl.header.cnt_songs)
                current_state.type = redraw_type_t::not_easy; // TODO
            else
                current_state.type = redraw_type_t::top_bottom;
        }
        current_state.pos = (current_state.pos + 1) % lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (current_state.pos + 1 == lpl.header.cnt_songs) //at end 
    {
        if ((ret = lpl.seek(0)))
            return ret;
        if ((ret = fill_names()))
            return ret;
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = 0;
        pos_begin = 0;
        return 0;
    }
    
    if ((current_state.pos < playlist_border_cnt) || // on top
        (current_state.pos + playlist_border_cnt + 1 >= lpl.header.cnt_songs)) // on bottom
    {
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::top_bottom;
        current_state.pos++;
    }
    else
    {
        if ((ret = lpl.seek(current_state.pos + playlist_border_cnt + 1)))
            return ret;
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::middle;
        current_state.pos++;
        pos_begin++;
        size_t ins_pos = pos_begin + playlist_view_cnt - 1;
        pn_common(ins_pos);
    }
    
    return 0;
}

uint32_t playlist_view::prev ()
{
    current_state.direction = 1;
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        if (current_state.type == redraw_type_t::nothing)
        {
            if (current_state.pos == 0)
                current_state.type = redraw_type_t::not_easy; // TODO
            else
                current_state.type = redraw_type_t::top_bottom;
        }
        current_state.pos = (current_state.pos + lpl.header.cnt_songs - 1) % lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (current_state.pos == 0) // at begin
    {
        if ((ret = lpl.seek(lpl.header.cnt_songs - playlist_view_cnt)))
            return ret;
        if ((ret = fill_names()))
            return ret;
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = lpl.header.cnt_songs - 1;
        pos_begin = 0;
        return 0;
    }
    
    if ((current_state.pos <= playlist_border_cnt) || // on top
        (current_state.pos + playlist_border_cnt >= lpl.header.cnt_songs)) // on bottom
    {
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::top_bottom;
        current_state.pos--;
    }
    else
    {
        if ((ret = lpl.seek(current_state.pos - playlist_border_cnt - 1)))
            return ret;
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::middle;
        pos_begin = pos_begin + playlist_view_cnt - 1;
        current_state.pos--;
        size_t ins_pos = pos_begin;
        pn_common(ins_pos);
    }
    
    return 0;
}

uint32_t playlist_view::play (playlist & pl) const
{
    return pl.open(lpl, current_state.pos);
}

bool playlist_view::compare (playlist const & b) const
{
    return lpl.fd == b.lpl.fd;
}

bool playlist_view::check_near (playlist const & playing_pl) const
{
    if (!compare(playing_pl))
        return 0;
    
    return ::check_near
    (
        current_state.pos, 
        playing_pl.lpl.pos, 
        lpl.header.cnt_songs, 
        playlist_view_cnt, 
        playlist_border_cnt
    );
}

void playlist_view::print
(
    playlist const & playing_pl,
    char (* song_name)[sz::song_name + 1], 
    char (* group_name)[sz::group_name + 1], 
    char * selected,
    char (* number)[3 + 1]
) const
{
    memset(selected, 0, playlist_view_cnt);
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        if (lpl.header.cnt_songs != 0)
            selected[current_state.pos] |= 1;
        for (size_t i = 0; i != lpl.header.cnt_songs; ++i)
        {
            memcpy(song_name[i], this->name_song[i + pos_begin], sz::song_name + 1);
            memcpy(group_name[i], this->name_group[i + pos_begin], sz::group_name + 1);
            snprintf(number[i], 3 + 1, "%3u", (i % 1000));
        }
        for (size_t i = lpl.header.cnt_songs; i != playlist_view_cnt; ++i)
        {
            memset(song_name[i], ' ', sz::song_name);
            memset(group_name[i], ' ', sz::group_name);
            memset(number[i], ' ', 3);
            song_name[i][sz::song_name] = 0;
            group_name[i][sz::group_name] = 0;
            number[i][3] = 0;
        }
        
        if ((compare(playing_pl)) &&
            (lpl.header.cnt_songs != 0))
        {
            selected[playing_pl.lpl.pos] |= 2;
        }
        return;
    }
    
    uint32_t index;
    if (current_state.pos < playlist_border_cnt) // on top
    {
        index = 0;
        selected[current_state.pos] |= 1;
    }
    else if (current_state.pos + playlist_border_cnt >= lpl.header.cnt_songs) // on bottom
    {
        index = lpl.header.cnt_songs - playlist_view_cnt;
        selected[current_state.pos + playlist_view_cnt - lpl.header.cnt_songs] |= 1;
    }
    else // on middle
    {
        index = current_state.pos - playlist_border_cnt;
        selected[playlist_border_cnt] |= 1;
    }

    if (check_near(playing_pl))
        selected[playing_pl.lpl.pos - index] |= 2;

    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        memcpy(song_name[i], this->name_song[(i + pos_begin) % playlist_view_cnt], sz::song_name + 1);
        memcpy(group_name[i], this->name_group[(i + pos_begin) % playlist_view_cnt], sz::group_name + 1);
        snprintf(number[i], 3 + 1, "%3lu", ((index + i) % 1000));
    }
}

uint32_t playlist_view::to_playing_playlist (playlist const & pl)
{
    file_descriptor old_fd(lpl.fd);
    
    lpl.fd.copy_seek_0(pl.lpl.fd);
    uint32_t ret;
    if ((ret = init()))
    {
        lpl.fd = old_fd;
        return ret;
    }
    return 0;
}

uint32_t playlist_view::open_playlist
(
    char const (* const path)[12],
    uint32_t path_len
)
{
    uint32_t ret;
    file_descriptor fd(lpl.fd);
    if ((ret = open(&FAT_info, &lpl.fd, path, path_len)))
        return ret;
    if ((ret = init()))
    {
        lpl.fd = fd;
        return ret;
    }
    return 0;
}

bool playlist_view::is_fake () const
{
    return lpl.fd.is_fake();
}


void playlist_view::reset_display ()
{
    current_state.type = redraw_type_t::nothing;
}

redraw_type_t playlist_view::redraw_type () const
{
    redraw_type_t ans = current_state;

    if (current_state.pos <= playlist_border_cnt) // on top
        ans.pos = current_state.pos;
    else if (current_state.pos + playlist_border_cnt >= lpl.header.cnt_songs) // on bottom
        ans.pos = current_state.pos + playlist_view_cnt - lpl.header.cnt_songs;
    else // on middle
        ans.pos = playlist_border_cnt;
    return ans;
}

