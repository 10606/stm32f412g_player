#include "view.h"

#include "display.h"

uint32_t init_view (view * vv, char (* path)[12], uint32_t len, audio_ctl_t * audio_ctl)
{
    vv->audio_ctl = audio_ctl;
    vv->state = D_PL_LIST;
    vv->old_state = D_PL_LIST;
    vv->state_song_view = S_VOLUME;
    vv->playing_playlist = max_plb_files;
    vv->selected_playlist = max_plb_files;
    vv->pl.fd = &vv->fd_pl;
    
    init_fake_file_descriptor(&vv->fd_plv);
    init_fake_file_descriptor(&vv->fd_pl);
    
    uint32_t ret;
    ret = init_pl_list(&vv->pll, path, len);
    if (ret)
        return ret;

    ret = init_playlist_view(&vv->plv, &vv->fd_plv);
    if (ret)
        return ret;
    
    ret = play(&vv->plv, &vv->pl);
    if (ret)
        return ret;
    vv->playing_playlist = vv->selected_playlist;
    return 0;
}

uint32_t destroy_view (view * vv)
{
    destroy_pl_list(&vv->pll);
    destroy_playlist(&vv->pl);
    return 0;
}

void display_view (view * vv, uint8_t * need_redraw)
{
    char ts_playlist = (vv->state == D_PLAYLIST);
    char ts_pl_list = (vv->state == D_PL_LIST);
    char ts_song = (vv->state == D_SONG);

    display_playlist(&vv->plv, &vv->pl, vv->state, ts_playlist, need_redraw);
    display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, ts_pl_list, need_redraw);
    display_song(&vv->pl, vv->audio_ctl, &vv->state_song_view, ts_song, (vv->old_state != D_SONG), need_redraw);
    vv->old_state = vv->state;
}

uint32_t open_song (view * vv)
{
    if (vv->pl.header.cnt_songs == 0)
    {
        init_fake_file_descriptor(&vv->audio_ctl->audio_file);
        vv->audio_ctl->info.offset = 0;
        return 0;
    }

    uint32_t ret;
    if ((ret = open(&vv->audio_ctl->audio_file, vv->pl.path, vv->pl.song.path_len)))
        return ret;
    vv->audio_ctl->seeked = 1;
    get_length(&vv->audio_ctl->audio_file, &vv->audio_ctl->info);
    if ((ret = f_seek(&vv->audio_ctl->audio_file, vv->audio_ctl->info.offset)))
        return ret;
    return 0;
}

uint32_t open_song_not_found (view * vv, char direction)
{
    uint32_t ret = 0;
    for (uint32_t i = 0; i != vv->pl.header.cnt_songs; direction? --i : ++i)
    {
        if ((ret = open_song(vv)))
        {
            if (ret == not_found)
                continue;
            else
                return ret;
        }
        else
        {
            return 0;
        }
        
        if ((ret = next_playlist(&vv->pl)))
            return ret;
    }
    return ret;
}

void fake_song_and_playlist (view * vv)
{
    destroy_playlist(&vv->pl);
    init_fake_file_descriptor(&vv->fd_pl);
    init_playlist(&vv->pl, &vv->fd_pl); // noexcept if fake_file_descriptor
    
    init_fake_file_descriptor(&vv->audio_ctl->audio_file);
}

