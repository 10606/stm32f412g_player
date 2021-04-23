#include "playlist_view.h"

#include "util.h"
#include <stdio.h>

uint32_t playlist_view::fill_names ()
{
    if (lpl.header.cnt_songs == 0)
        return 0;

    char name_group_backup[playlist_view_cnt][group_name_sz + 1];
    char name_song_backup[playlist_view_cnt][song_name_sz + 1];
    memcpy(name_group_backup, name_group, sizeof(name_group));
    memcpy(name_song_backup, name_song, sizeof(name_song));
    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        memcpy(name_group[i], lpl.song.group_name, group_name_sz);
        name_group[i][group_name_sz] = 0;
        memcpy(name_song[i], lpl.song.song_name, song_name_sz);
        name_song[i][song_name_sz] = 0;
        if (i + 1 == lpl.header.cnt_songs)
            break;
        uint32_t ret;
        ret = lpl.next(); 
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
    pos_selected = 0;
    return 0;
}

void playlist_view::pn_common (size_t ins_pos)
{
    pos_begin %= lpl.header.cnt_songs;
    pos_begin %= playlist_view_cnt;
    
    ins_pos %= lpl.header.cnt_songs;
    ins_pos %= playlist_view_cnt;
    
    memcpy(name_group[ins_pos], lpl.song.group_name, group_name_sz);
    name_group[ins_pos][group_name_sz] = 0;
    memcpy(name_song[ins_pos], lpl.song.song_name, song_name_sz);
    name_song[ins_pos][song_name_sz] = 0;
}

uint32_t playlist_view::seek (uint32_t pos)
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    pos = pos % lpl.header.cnt_songs;

    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        pos_selected = pos;
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
    pos_selected = pos;
    
    return 0;
}

uint32_t playlist_view::down ()
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        pos_selected = (pos_selected + 1) % lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (pos_selected + 1 == lpl.header.cnt_songs) //at end 
    {
        pos_selected = 0;
        pos_begin = 0;
        if ((ret = lpl.seek(0)))
            return ret;
        if ((ret = fill_names()))
            return ret;
        return 0;
    }
    
    if (pos_selected < playlist_border_cnt)
        pos_selected++;
    else if (pos_selected + playlist_border_cnt + 1 >= lpl.header.cnt_songs)
        pos_selected++;
    else
    {
        if ((ret = lpl.seek(pos_selected + playlist_border_cnt + 1)))
            return ret;
        pos_selected++;
        pos_begin++;
        size_t ins_pos = pos_begin + playlist_view_cnt - 1;
        pn_common(ins_pos);
    }
    
    return 0;
}

uint32_t playlist_view::up ()
{
    if (lpl.header.cnt_songs == 0)
        return 0;
    
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        pos_selected = (pos_selected + lpl.header.cnt_songs - 1) % lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (pos_selected == 0) // at begin
    {
        pos_selected = lpl.header.cnt_songs - 1;
        pos_begin = 0;
        if ((ret = lpl.seek(lpl.header.cnt_songs - playlist_view_cnt)))
            return ret;
        if ((ret = fill_names()))
            return ret;
        return 0;
    }
    
    if (pos_selected <= playlist_border_cnt)
        pos_selected--;
    else if (pos_selected + playlist_border_cnt >= lpl.header.cnt_songs)
        pos_selected--;
    else
    {
        if ((ret = lpl.seek(pos_selected - playlist_border_cnt - 1)))
            return ret;
        pos_begin = pos_begin + playlist_view_cnt - 1;
        pos_selected--;
        size_t ins_pos = pos_begin;
        pn_common(ins_pos);
    }
    
    return 0;
}

uint32_t playlist_view::play (playlist * pl) 
{
    return pl->set_file(&lpl.fd, pos_selected);
}

bool playlist_view::compare (playlist const * b)
{
    return lpl.fd == b->fd;
}

bool playlist_view::check_near (playlist * playing_pl)
{
    if (!compare(playing_pl))
        return 0;
    
    return ::check_near
    (
        pos_selected, 
        playing_pl->pos, 
        lpl.header.cnt_songs, 
        playlist_view_cnt, 
        playlist_border_cnt
    );
}

void playlist_view::print
(
    playlist * playing_pl,
    char (* song_name)[song_name_sz + 1], 
    char (* group_name)[group_name_sz + 1], 
    char * selected,
    char (* number)[3 + 1]
)
{
    memset(selected, 0, playlist_view_cnt);
    if (lpl.header.cnt_songs <= playlist_view_cnt)
    {
        if (lpl.header.cnt_songs != 0)
            selected[pos_selected] |= 1;
        for (size_t i = 0; i != lpl.header.cnt_songs; ++i)
        {
            memcpy(song_name[i], this->name_song[i + pos_begin], song_name_sz + 1);
            memcpy(group_name[i], this->name_group[i + pos_begin], group_name_sz + 1);
            snprintf(number[i], 3 + 1, "%3u", (i % 1000));
        }
        for (size_t i = lpl.header.cnt_songs; i != playlist_view_cnt; ++i)
        {
            memset(song_name[i], ' ', song_name_sz);
            memset(group_name[i], ' ', group_name_sz);
            memset(number[i], ' ', 3);
            song_name[i][song_name_sz] = 0;
            group_name[i][group_name_sz] = 0;
            number[i][3] = 0;
        }
        
        if ((compare(playing_pl)) &&
            (lpl.header.cnt_songs != 0))
        {
            selected[playing_pl->pos] |= 2;
        }
        return;
    }
    
    uint32_t index;
    if (pos_selected < playlist_border_cnt) // on top
    {
        index = 0;
        selected[pos_selected] |= 1;
    }
    else if (pos_selected + playlist_border_cnt >= lpl.header.cnt_songs) // on bottom
    {
        index = lpl.header.cnt_songs - playlist_view_cnt;
        selected[pos_selected + playlist_view_cnt - lpl.header.cnt_songs] |= 1;
    }
    else // on middle
    {
        index = pos_selected - playlist_border_cnt;
        selected[playlist_border_cnt] |= 1;
    }

    if (check_near(playing_pl))
        selected[playing_pl->pos - index] |= 2;

    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        memcpy(song_name[i], this->name_song[(i + pos_begin) % playlist_view_cnt], song_name_sz + 1);
        memcpy(group_name[i], this->name_group[(i + pos_begin) % playlist_view_cnt], group_name_sz + 1);
        snprintf(number[i], 3 + 1, "%3lu", ((index + i) % 1000));
    }
    
}

uint32_t playlist_view::to_playing_playlist (playlist const & pl)
{
    file_descriptor old_fd(lpl.fd);
    
    lpl.fd.copy_seek_0(pl.fd);
    uint32_t ret;
    if ((ret = init()))
    {
        lpl.fd.copy(old_fd);
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
    // trivially destructible
    if ((ret = init()))
    {
        lpl.fd.copy(fd);
        return ret;
    }
    return 0;
}

bool playlist_view::is_fake ()
{
    return lpl.fd.is_fake();
}

