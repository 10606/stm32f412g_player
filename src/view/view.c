#include "view.h"

#include "util.h"
#include "joystick.h"
#include "stm32412g_discovery_audio.h"
#include "display.h"

void init_mad ();
void deinit_mad ();

uint32_t seek_value = (1024 * 32);

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
    char ts_playlist = 0;
    char ts_pl_list = 0;
    char ts_song = 0;
    switch (vv->state)
    {
    case D_PL_LIST:
        ts_pl_list = 1;
        break;

    case D_PLAYLIST:
        ts_playlist = 1;
        break;
        
    case D_SONG:
        ts_song = 1;
        break;

    default:
        break;
    }

    display_playlist(&vv->plv, &vv->pl, vv->state, ts_playlist, need_redraw);
    display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, ts_pl_list, need_redraw);
    display_song(&vv->pl, vv->audio_ctl, &vv->state_song_view, ts_song, (vv->old_state != D_SONG), need_redraw);
    vv->old_state = vv->state;
}

uint32_t process_view_change_volume (view * vv, uint8_t * need_redraw, int32_t value)
{
    if (vv->audio_ctl->volume + value > 100)
        vv->audio_ctl->volume = 100;
    else if (vv->audio_ctl->volume + value < 0)
        vv->audio_ctl->volume = 0;
    else 
        vv->audio_ctl->volume += value;
    BSP_AUDIO_OUT_SetVolume(vv->audio_ctl->volume);
    display_song_volume(&vv->pl, vv->audio_ctl, &vv->state_song_view, (vv->state == D_SONG), need_redraw);
    return 0;
}

uint32_t process_view_inc_volume (view * vv, uint8_t * need_redraw)
{
    return process_view_change_volume(vv, need_redraw, 1);
}

uint32_t process_view_dec_volume (view * vv, uint8_t * need_redraw)
{
    return process_view_change_volume(vv, need_redraw, -1);
}

uint32_t process_view_seek (view * vv, uint8_t * need_redraw, uint32_t value, uint8_t direction /* 0 - backward, 1 - forward */)
{
    uint32_t ret, new_pos;
    new_pos = current_position(&vv->audio_ctl->audio_file);
    new_pos = direction? 
        add_in_bound(new_pos, 0, vv->audio_ctl->audio_file.size, value) :
        sub_in_bound(new_pos, 0, vv->audio_ctl->audio_file.size, value);
    vv->audio_ctl->seeked = 1;
    ret = f_seek(&vv->audio_ctl->audio_file, new_pos);
    if (ret)
        return ret;
    return 0;
}

uint32_t process_view_seek_forward (view * vv, uint8_t * need_redraw)
{
    return process_view_seek(vv, need_redraw, seek_value, 1);
}

uint32_t process_view_seek_backward (view * vv, uint8_t * need_redraw)
{
    return process_view_seek(vv, need_redraw, seek_value, 0);
}

uint32_t process_view_change_song (view * vv, uint8_t * need_redraw, uint8_t direction /* 0 - next, 1 - prev */)
{
    static uint32_t (* const do_on_playlist[2]) (playlist *) =
    {
        next_playlist,
        prev_playlist
    };
    uint32_t ret;
    ret = do_on_playlist[direction](&vv->pl);
    if (ret)
        return ret;
    else
    {
        deinit_mad();
        init_mad();
        if ((ret = open_song_not_found(vv, direction))) 
            return ret;
    }
    *need_redraw = 1;
    return 0;
}

uint32_t process_view_prev_song (view * vv, uint8_t * need_redraw)
{
    return process_view_change_song(vv, need_redraw, 1);
}

uint32_t process_view_next_song (view * vv, uint8_t * need_redraw)
{
    return process_view_change_song(vv, need_redraw, 0);
}

uint32_t process_view_up_down (view * vv, uint8_t * need_redraw, uint8_t direction /* 0 - down, 1 - up */)
{
    static void (* const do_on_pl_list[2]) (pl_list *) = 
    {
        down_pl_list,
        up_pl_list
    };
    static uint32_t (* const do_on_playlist_view[2]) (playlist_view *) = 
    {
        down,
        up
    };
    
    switch (vv->state)
    {
    case D_PL_LIST:
        do_on_pl_list[direction](&vv->pll);
        *need_redraw = 1;
        break;

    case D_PLAYLIST:
        *need_redraw = 1;
        return do_on_playlist_view[direction](&vv->plv);
        
    case D_SONG:
        switch (vv->state_song_view)
        {
        case S_VOLUME:
            return process_view_change_volume(vv, need_redraw, direction? 1 : -1);
            
        case S_SEEK:
            return process_view_seek(vv, need_redraw, seek_value, direction);

        case S_NEXT_PREV:
            return process_view_change_song(vv, need_redraw, direction);
        }
    }
    return 0;
}

uint32_t process_view_up (view * vv, uint8_t * need_redraw)
{
    return process_view_up_down(vv, need_redraw, 1);
}

uint32_t process_view_down (view * vv, uint8_t * need_redraw)
{
    return process_view_up_down(vv, need_redraw, 0);
}

uint32_t process_view_play_pause (view * vv, uint8_t * need_redraw)
{
    if (vv->audio_ctl->pause_status == 1)
    {
        BSP_AUDIO_OUT_Resume();
        vv->audio_ctl->pause_status = 0;
    }
    else
    {
        BSP_AUDIO_OUT_Pause();
        vv->audio_ctl->pause_status = 1;
    }
    *need_redraw = 1;
    return 0;
}


uint32_t process_view_left (view * vv, uint8_t * need_redraw)
{
    switch (vv->state)
    {
    case D_PL_LIST:
        break;

    case D_PLAYLIST:
        vv->state = D_PL_LIST;
        *need_redraw = 1;
        break;
        
    case D_SONG:
        vv->state = D_PLAYLIST;
        *need_redraw = 1;
        break;
    }
    return 0;
}

static inline uint32_t play_new_playlist (view * vv)
{
    uint32_t ret;
    file_descriptor old_fd;
    playlist old_pl;
    copy_file_descriptor(&old_fd, vv->pl.fd);
    move_playlist(&old_pl, &vv->pl);

    ret = play(&vv->plv, &vv->pl);
    if (ret)
        return ret;
    ret = open_song_not_found(vv, 0);
    if (ret)
    {
        copy_file_descriptor(vv->pl.fd, &old_fd);
        move_playlist(&vv->pl, &old_pl);
        destroy_playlist(&old_pl);
        return ret;
    }

    vv->playing_playlist = vv->selected_playlist;
    deinit_mad();
    init_mad();
    return 0;
}

uint32_t process_view_right (view * vv, uint8_t * need_redraw)
{
    uint32_t ret;
    switch (vv->state)
    {
    case D_PL_LIST:
        vv->state = D_PLAYLIST;
        ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist);
        if (ret)
            return ret;
        *need_redraw = 1;
        break;

    case D_PLAYLIST:
        if ((ret = play_new_playlist(vv)))
            return ret;
        *need_redraw = 1;
        break;
        
    case D_SONG:
        vv->state_song_view = ((vv->state_song_view + 1) % state_song_view_cnt);
        display_song_volume(&vv->pl, vv->audio_ctl, &vv->state_song_view, 1, need_redraw);
        break;
    }
    return 0;
}

static inline uint32_t view_to_playing_playlist (view * vv, uint8_t * need_redraw)
{
    if (vv->pl.header.cnt_songs != 0)
    {
        uint32_t ret;
        if (!playlist_compare(&vv->plv.lpl, &vv->pl))
        {
            file_descriptor old_fd;
            copy_file_descriptor(&old_fd, vv->plv.lpl.fd);
            
            copy_file_descriptor_seek_0(vv->plv.lpl.fd, vv->pl.fd);
            // trivially destrucible
            if ((ret = init_playlist_view(&vv->plv, vv->plv.lpl.fd)))
            {
                copy_file_descriptor(vv->plv.lpl.fd, &old_fd);
                return ret;
            }
        }
        ret = seek_playlist_view(&vv->plv, vv->pl.pos);
        if (ret)
            return ret;
        *need_redraw = 1;
    }
    return 0;
}

uint32_t process_toggle_repeat (view * vv, uint8_t * need_redraw)
{
    vv->audio_ctl->repeat_mode ^= 1;
    display_song_volume(&vv->pl, vv->audio_ctl, &vv->state_song_view, 1, need_redraw);
    return 0;
}

uint32_t process_view_center (view * vv, uint8_t * need_redraw)
{
    switch (vv->state)
    {
    case D_PL_LIST:
        if (pl_list_check_near(&vv->pll, vv->playing_playlist))
            vv->state = D_PLAYLIST;
        else
            seek_pl_list(&vv->pll, vv->playing_playlist);
        *need_redraw = 1;
        break;
        
    case D_PLAYLIST:
        if (playlist_check_near(&vv->plv, &vv->pl))
        {
            vv->state = D_SONG;
            *need_redraw = 1;
        }
        else
        {
            view_to_playing_playlist(vv, need_redraw);
        }
        break;
        
    case D_SONG:
        process_toggle_repeat(vv, need_redraw);
        break;
    }
    return 0;
}

static inline char check_button_state (uint32_t joy_button)
{
    static uint8_t const cost[3] = {1, 8, 1}; // first second next
    char ans = joystick_state.process[joy_button] >= cost[joystick_state.prev_processed[joy_button]];
    if (ans)
    {
        joystick_state.process[joy_button] -= cost[joystick_state.prev_processed[joy_button]];
        joystick_state.prev_processed[joy_button]++;
        if (joystick_state.prev_processed[joy_button] > 2)
            joystick_state.prev_processed[joy_button] = 2;
    }
    return ans;
}

uint32_t process_view (view * vv, uint8_t * need_redraw)
{
    static uint32_t (* const process_view_do[joystick_states_cnt]) (view *, uint8_t *) = 
    {
        process_view_up,
        process_view_down,
        process_view_left,
        process_view_right,
        process_view_center
    };
    static enum joystick_buttons buttons[joystick_states_cnt] = 
    {
        joy_button_up,
        joy_button_down,
        joy_button_left,
        joy_button_right,
        joy_button_center
    };
    char flag = 1;
    uint32_t ret = 0;
    while (flag)
    {
        flag = 0;
        for (uint32_t i = 0; i != joystick_states_cnt; ++i)
        {
            if (check_button_state(buttons[i]))
            {
                flag = 1;
                ret = process_view_do[i](vv, need_redraw);
                if (ret)
                    return ret;
            }
        }
    }
    return ret;
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

