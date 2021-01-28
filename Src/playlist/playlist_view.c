#include "playlist_view.h"

#include "util.h"
#include <stdio.h>

static uint32_t fill_names (playlist_view * plv)
{
    if (plv->lpl.header.cnt_songs == 0)
        return 0;

    char name_group[playlist_view_cnt][group_name_sz + 1];
    char name_song[playlist_view_cnt][song_name_sz + 1];
    memcpy(name_group, plv->name_group, sizeof(name_group));
    memcpy(name_song, plv->name_song, sizeof(name_song));
    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        memcpy(plv->name_group[i], plv->lpl.song.group_name, group_name_sz);
        plv->name_group[i][group_name_sz] = 0;
        memcpy(plv->name_song[i], plv->lpl.song.song_name, song_name_sz);
        plv->name_song[i][song_name_sz] = 0;
        if (i + 1 == plv->lpl.header.cnt_songs)
            break;
        uint32_t ret;
        ret = next_light_playlist(&plv->lpl); 
        if (ret)
        {
            memcpy(plv->name_group, name_group, sizeof(name_group));
            memcpy(plv->name_song, name_song, sizeof(name_song));
            return ret;
        }
    }
    return 0;
}

uint32_t init_playlist_view (playlist_view * plv, file_descriptor * fd)
{
    uint32_t ret;
    light_playlist old_lpl;
    memcpy(&old_lpl, &plv->lpl, sizeof(light_playlist));
    ret = init_light_playlist(&plv->lpl, fd);
    if (ret)
        return ret;
    ret = fill_names(plv);
    if (ret)
    {
        memcpy(&plv->lpl, &old_lpl, sizeof(light_playlist));
        return ret;
    }
    plv->pos_begin = 0;
    plv->pos_selected = 0;
    return 0;
}

static void _pn_common (playlist_view * plv, size_t ins_pos)
{
    plv->pos_begin %= plv->lpl.header.cnt_songs;
    plv->pos_begin %= playlist_view_cnt;
    
    ins_pos %= plv->lpl.header.cnt_songs;
    ins_pos %= playlist_view_cnt;
    
    memcpy(plv->name_group[ins_pos], plv->lpl.song.group_name, group_name_sz);
    plv->name_group[ins_pos][group_name_sz] = 0;
    memcpy(plv->name_song[ins_pos], plv->lpl.song.song_name, song_name_sz);
    plv->name_song[ins_pos][song_name_sz] = 0;
}

uint32_t seek_playlist_view (playlist_view * plv, uint32_t pos)
{
    if (plv->lpl.header.cnt_songs == 0)
        return 0;
    
    pos = pos % plv->lpl.header.cnt_songs;

    if (plv->lpl.header.cnt_songs <= playlist_view_cnt)
    {
        plv->pos_selected = pos;
        return 0;
    }
    
    uint32_t seek_pos;
    if (pos < playlist_border_cnt) // on top
        seek_pos = 0;
    else if (pos + playlist_border_cnt >= plv->lpl.header.cnt_songs) // on bottom
        seek_pos = plv->lpl.header.cnt_songs - playlist_view_cnt;
    else // on middle
        seek_pos = pos - playlist_border_cnt;

    light_playlist old_lpl;
    memcpy(&old_lpl, &plv->lpl, sizeof(light_playlist));
    uint32_t ret;
    if ((ret = seek_light_playlist(&plv->lpl, seek_pos)))
        return ret;
    if ((ret = fill_names(plv)))
    {
        memcpy(&plv->lpl, &old_lpl, sizeof(light_playlist));
        return ret;
    }
    plv->pos_begin = 0;
    plv->pos_selected = pos;
    
    return 0;
}

uint32_t down (playlist_view * plv)
{
    if (plv->lpl.header.cnt_songs == 0)
        return 0;
    
    if (plv->lpl.header.cnt_songs <= playlist_view_cnt)
    {
        plv->pos_selected = (plv->pos_selected + 1) % plv->lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (plv->pos_selected + 1 == plv->lpl.header.cnt_songs) //at end 
    {
        plv->pos_selected = 0;
        plv->pos_begin = 0;
        if ((ret = seek_light_playlist(&plv->lpl, 0)))
            return ret;
        if ((ret = fill_names(plv)))
            return ret;
        return 0;
    }
    
    if (plv->pos_selected < playlist_border_cnt)
        plv->pos_selected++;
    else if (plv->pos_selected + playlist_border_cnt + 1 >= plv->lpl.header.cnt_songs)
        plv->pos_selected++;
    else
    {
        if ((ret = seek_light_playlist(&plv->lpl, plv->pos_selected + playlist_border_cnt + 1)))
            return ret;
        plv->pos_selected++;
        plv->pos_begin++;
        size_t ins_pos = plv->pos_begin + playlist_view_cnt - 1;
        _pn_common(plv, ins_pos);
    }
    
    return 0;
}

uint32_t up (playlist_view * plv)
{
    if (plv->lpl.header.cnt_songs == 0)
        return 0;
    
    if (plv->lpl.header.cnt_songs <= playlist_view_cnt)
    {
        plv->pos_selected = (plv->pos_selected + plv->lpl.header.cnt_songs - 1) % plv->lpl.header.cnt_songs;
        return 0;
    }
    
    uint32_t ret;
    if (plv->pos_selected == 0) // at begin
    {
        plv->pos_selected = plv->lpl.header.cnt_songs - 1;
        plv->pos_begin = 0;
        if ((ret = seek_light_playlist(&plv->lpl, plv->lpl.header.cnt_songs - playlist_view_cnt)))
            return ret;
        if ((ret = fill_names(plv)))
            return ret;
        return 0;
    }
    
    if (plv->pos_selected <= playlist_border_cnt)
        plv->pos_selected--;
    else if (plv->pos_selected + playlist_border_cnt >= plv->lpl.header.cnt_songs)
        plv->pos_selected--;
    else
    {
        if ((ret = seek_light_playlist(&plv->lpl, plv->pos_selected - playlist_border_cnt - 1)))
            return ret;
        plv->pos_begin = plv->pos_begin + playlist_view_cnt - 1;
        plv->pos_selected--;
        size_t ins_pos = plv->pos_begin;
        _pn_common(plv, ins_pos);
    }
    
    return 0;
}

uint32_t play (playlist_view * plv, playlist * pl) 
{
    uint32_t ret;
    file_descriptor old_fd;
    playlist old_pl;
    copy_file_descriptor(&old_fd, pl->fd);
    move_playlist(&old_pl, pl);
    file_descriptor * pl_fd = pl->fd;
    destroy_playlist(pl);
    copy_file_descriptor_seek_0(pl_fd, plv->lpl.fd);
    if ((ret = init_playlist(pl, pl_fd)) == 0)
    {
        if ((ret = seek_playlist(pl, plv->pos_selected)) == 0)
        {
            destroy_playlist(&old_pl);
            return 0;
        }
    }

    copy_file_descriptor(pl->fd, &old_fd);
    destroy_playlist(pl);
    move_playlist(pl, &old_pl);
    destroy_playlist(&old_pl);
    return ret;
}

char playlist_compare (light_playlist * a, playlist * b)
{
    return eq_file_descriptor(a->fd, b->fd);
}

char playlist_check_near (playlist_view * plv, playlist * playing_pl)
{
    if (!playlist_compare(&plv->lpl, playing_pl))
        return 0;
    
    return check_near
    (
        plv->pos_selected, 
        playing_pl->pos, 
        plv->lpl.header.cnt_songs, 
        playlist_view_cnt, 
        playlist_border_cnt
    );
}

void print_playlist_view 
(
    playlist_view * plv, 
    playlist * playing_pl,
    char (* restrict song_name)[song_name_sz + 1], 
    char (* restrict group_name)[group_name_sz + 1], 
    char * restrict selected,
    char (* restrict number)[3 + 1]
)
{
    memset(selected, 0, playlist_view_cnt);
    if (plv->lpl.header.cnt_songs <= playlist_view_cnt)
    {
        if (plv->lpl.header.cnt_songs != 0)
            selected[plv->pos_selected] |= 1;
        for (size_t i = 0; i != plv->lpl.header.cnt_songs; ++i)
        {
            memcpy(song_name[i], plv->name_song[i + plv->pos_begin], song_name_sz + 1);
            memcpy(group_name[i], plv->name_group[i + plv->pos_begin], group_name_sz + 1);
            snprintf(number[i], 3 + 1, "%3u", (i % 1000));
        }
        for (size_t i = plv->lpl.header.cnt_songs; i != playlist_view_cnt; ++i)
        {
            memset(song_name[i], ' ', song_name_sz);
            memset(group_name[i], ' ', group_name_sz);
            memset(number[i], ' ', 3);
            song_name[i][song_name_sz] = 0;
            group_name[i][group_name_sz] = 0;
            number[i][3] = 0;
        }
        
        if ((playlist_compare(&plv->lpl, playing_pl)) &&
            (plv->lpl.header.cnt_songs != 0))
        {
            selected[playing_pl->pos] |= 2;
        }
        return;
    }
    
    uint32_t index;
    if (plv->pos_selected < playlist_border_cnt) // on top
    {
        index = 0;
        selected[plv->pos_selected] |= 1;
    }
    else if (plv->pos_selected + playlist_border_cnt >= plv->lpl.header.cnt_songs) // on bottom
    {
        index = plv->lpl.header.cnt_songs - playlist_view_cnt;
        selected[plv->pos_selected + playlist_view_cnt - plv->lpl.header.cnt_songs] |= 1;
    }
    else // on middle
    {
        index = plv->pos_selected - playlist_border_cnt;
        selected[playlist_border_cnt] |= 1;
    }

    if (playlist_check_near(plv, playing_pl))
        selected[playing_pl->pos - index] |= 2;

    for (size_t i = 0; i != playlist_view_cnt; ++i)
    {
        memcpy(song_name[i], plv->name_song[(i + plv->pos_begin) % playlist_view_cnt], song_name_sz + 1);
        memcpy(group_name[i], plv->name_group[(i + plv->pos_begin) % playlist_view_cnt], group_name_sz + 1);
        snprintf(number[i], 3 + 1, "%3lu", ((index + i) % 1000));
    }
    
}

void bind_playlist_view (playlist_view * plv, file_descriptor * fd)
{
    plv->lpl.fd = fd;
}

uint32_t open_playlist
(
    playlist_view * plv,
    char const (* const path)[12],
    uint32_t path_len
)
{
    uint32_t ret;
    file_descriptor fd;
    copy_file_descriptor(&fd, plv->lpl.fd);
    if ((ret = open(plv->lpl.fd, path, path_len)))
        return ret;
    // trivially destructible
    if ((ret = init_playlist_view(plv, plv->lpl.fd)))
    {
        copy_file_descriptor(plv->lpl.fd, &fd);
        return ret;
    }
    return 0;
}

char is_fake_playlist_view (playlist_view * plv)
{
    return is_fake_file_descriptor(plv->lpl.fd);
}

