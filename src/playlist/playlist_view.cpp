#include "playlist_view.h"

#include "util.h"
#include <stdio.h>
#include <type_traits>

ret_code playlist_view::fill_names (file_descriptor const & backup)
{
    if (lpl.header.cnt_songs == 0)
        return 0;

    decltype(name_group) name_group_backup;
    decltype(name_song)  name_song_backup;
    memcpy(name_group_backup, name_group, sizeof(name_group));
    memcpy(name_song_backup, name_song, sizeof(name_song));
    for (size_t i = 0; i != view_cnt; ++i)
    {
        copy_from_lpl(i);
        if (i + 1 == lpl.header.cnt_songs)
            break;
        ret_code ret = lpl.next(backup); 
        if (ret)
        {
            memcpy(name_group, name_group_backup, sizeof(name_group));
            memcpy(name_song, name_song_backup, sizeof(name_song));
            return ret;
        }
    }
    return 0;
}

ret_code playlist_view::init ()
{
    ret_code ret;
    light_playlist old_lpl(lpl);
    ret = lpl.open_file();
    if (ret)
        return ret;
    ret = fill_names(old_lpl.fd);
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

ret_code playlist_view::seek (uint32_t pos)
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    pos = pos % lpl.header.cnt_songs;

    if (lpl.header.cnt_songs <= view_cnt)
    {
        current_state.type = redraw_type_t::not_easy;
        current_state.pos = pos;
        return 0;
    }
    
    uint32_t seek_pos;
    if (pos < border_cnt) // on top
        seek_pos = 0;
    else if (pos + border_cnt >= lpl.header.cnt_songs) // on bottom
        seek_pos = lpl.header.cnt_songs - view_cnt;
    else // on middle
        seek_pos = pos - border_cnt;

    light_playlist old_lpl(lpl);
    ret_code ret;
    if ((ret = lpl.seek(seek_pos, old_lpl.fd)))
        return ret;
    if ((ret = fill_names(old_lpl.fd)))
    {
        lpl = old_lpl;
        return ret;
    }
    pos_begin = 0;
    current_state.pos = pos;
    current_state.type = redraw_type_t::not_easy;
    
    return 0;
}

ret_code playlist_view::jump_over (uint32_t seek_pos, uint32_t new_pos)
{
    ret_code ret;
    light_playlist old_lpl = lpl;
    if ((ret = lpl.seek(seek_pos)))
        return ret;
    if ((ret = fill_names(old_lpl.fd)))
        return ret;
    current_state.type = redraw_type_t::not_easy;
    current_state.pos = new_pos;
    pos_begin = 0;
    return 0;
}

ret_code playlist_view::next ()
{
    current_state.direction = 0;
    if (lpl.header.cnt_songs == 0)
        return 0;
    if (current_state.type != redraw_type_t::nothing)
        current_state.type = redraw_type_t::not_easy;
    
    if (lpl.header.cnt_songs <= view_cnt)
    {
        next_prev_for_short(1);
        return 0;
    }
    
    if (current_state.pos + 1 == lpl.header.cnt_songs) //at end 
        return jump_over(0, 0);
    
    if ((current_state.pos < border_cnt) || // on top
        (current_state.pos + border_cnt + 1 >= lpl.header.cnt_songs)) // on bottom
    {
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::top_bottom;
        current_state.pos++;
    }
    else
    {
        ret_code ret;
        if ((ret = lpl.seek(current_state.pos + border_cnt + 1)))
            return ret;
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::middle;
        current_state.pos++;
        pos_begin++;
        pn_common(pos_begin + view_cnt - 1);
    }
    
    return 0;
}

ret_code playlist_view::prev ()
{
    current_state.direction = 1;
    if (lpl.header.cnt_songs == 0)
        return 0;
    if (current_state.type != redraw_type_t::nothing)
        current_state.type = redraw_type_t::not_easy;
    
    if (lpl.header.cnt_songs <= view_cnt)
    {
        next_prev_for_short(lpl.header.cnt_songs - 1);
        return 0;
    }
    
    if (current_state.pos == 0) // at begin
        return jump_over(lpl.header.cnt_songs - view_cnt, lpl.header.cnt_songs - 1);
    
    if ((current_state.pos <= border_cnt) || // on top
        (current_state.pos + border_cnt >= lpl.header.cnt_songs)) // on bottom
    {
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::top_bottom;
        current_state.pos--;
    }
    else
    {
        ret_code ret;
        if ((ret = lpl.seek(current_state.pos - border_cnt - 1)))
            return ret;
        if (current_state.type == redraw_type_t::nothing)
            current_state.type = redraw_type_t::middle;
        current_state.pos--;
        pos_begin = pos_begin + view_cnt - 1;
        pn_common(pos_begin);
    }
    
    return 0;
}

ret_code playlist_view::play (playlist & pl, playlist const & backup) const
{
    return pl.open(lpl, current_state.pos, backup);
}

bool playlist_view::check_near (playlist const & playing_pl) const
{
    if (!compare(playing_pl.lpl))
        return 0;
    
    return ::check_near
    (
        current_state.pos, 
        playing_pl.lpl.pos, 
        lpl.header.cnt_songs, 
        view_cnt, 
        border_cnt
    );
}

ret_code playlist_view::to_playing_playlist (light_playlist const & pl)
{
    file_descriptor old_fd(lpl.fd);
    
    lpl.fd.copy_seek_0(pl.fd);
    ret_code ret;
    if ((ret = init()))
    {
        lpl.fd = old_fd;
        return ret;
    }
    return 0;
}

ret_code playlist_view::open_playlist
(
    filename_t const * path,
    uint32_t path_len
)
{
    ret_code ret;
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

    if (current_state.pos <= border_cnt) // on top
        ans.pos = current_state.pos;
    else if (current_state.pos + border_cnt >= lpl.header.cnt_songs) // on bottom
        ans.pos = current_state.pos + view_cnt - lpl.header.cnt_songs;
    else // on middle
        ans.pos = border_cnt;
    return ans;
}

light_playlist playlist_view::lpl_with_wrong_pos () const
{
    return lpl;
    // after this need 
    //  seek(plv.get_pos());
    // to get selected pos
}

