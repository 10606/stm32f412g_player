#include "view.h"

#include "stm32412g_discovery_audio.h"
#include "display.h"
#include "util.h"
#include "joystick.h"
#include "mp3.h"

inline state_t prev (state_t const & value)
{
    switch (value)
    {
    case state_t::pl_list:
        return state_t::pl_list;
    case state_t::playlist:
        return state_t::pl_list;
    case state_t::song:
        return state_t::playlist;
    }
}

inline state_t next (state_t const & value)
{
    switch (value)
    {
    case state_t::pl_list:
        return state_t::playlist;
    case state_t::playlist:
        return state_t::song;
    case state_t::song:
        return state_t::song;
    }
}

inline state_song_view_t roll (state_song_view_t const & value)
{
    switch (value)
    {
    case state_song_view_t::volume:
        return state_song_view_t::seek;
    case state_song_view_t::seek:
        return state_song_view_t::next_prev;
    case state_song_view_t::next_prev:
        return state_song_view_t::volume;
    }
}

uint32_t seek_value = (1024 * 32);

uint32_t process_view_change_volume (view * vv, uint8_t * need_redraw, int8_t value)
{
    if (value > 100)
        value = 100;
    if (value < -100)
        value = -100;
    if (vv->audio_ctl->volume + value > 100)
        vv->audio_ctl->volume = 100;
    else if (vv->audio_ctl->volume < -value)
        vv->audio_ctl->volume = 0;
    else
        vv->audio_ctl->volume += value;
    BSP_AUDIO_OUT_SetVolume(vv->audio_ctl->volume);
    display_song_volume(&vv->pl, vv->audio_ctl, vv->state_song_view, (vv->state == state_t::song), need_redraw);
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
        reuse_mad();
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
    case state_t::pl_list:
        do_on_pl_list[direction](&vv->pll);
        *need_redraw = 1;
        break;

    case state_t::playlist:
        *need_redraw = 1;
        return do_on_playlist_view[direction](&vv->plv);
        
    case state_t::song:
        switch (vv->state_song_view)
        {
        case state_song_view_t::volume:
            return process_view_change_volume(vv, need_redraw, direction? 1 : -1);
            
        case state_song_view_t::seek:
            return process_view_seek(vv, need_redraw, seek_value, direction);

        case state_song_view_t::next_prev:
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
    if (vv->state != state_t::pl_list)
    {
        vv->state = prev(vv->state);
        *need_redraw = 1;
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
    reuse_mad();
    return 0;
}

uint32_t process_view_right (view * vv, uint8_t * need_redraw)
{
    uint32_t ret;
    switch (vv->state)
    {
    case state_t::pl_list:
        vv->state = state_t::playlist;
        ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist);
        if (ret)
            return ret;
        *need_redraw = 1;
        break;

    case state_t::playlist:
        if ((ret = play_new_playlist(vv)))
            return ret;
        *need_redraw = 1;
        break;
        
    case state_t::song:
        vv->state_song_view = roll(vv->state_song_view);
        display_song_volume(&vv->pl, vv->audio_ctl, vv->state_song_view, 1, need_redraw);
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
            seek_pl_list(&vv->pll, vv->playing_playlist);
            vv->selected_playlist = vv->playing_playlist;
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
    display_song_volume(&vv->pl, vv->audio_ctl, vv->state_song_view, 1, need_redraw);
    return 0;
}

uint32_t process_view_center (view * vv, uint8_t * need_redraw)
{
    switch (vv->state)
    {
    case state_t::pl_list:
        if (pl_list_check_near(&vv->pll, vv->playing_playlist))
        {
            uint32_t ret = open_index_pl_list(&vv->pll, &vv->plv, vv->playing_playlist, &vv->selected_playlist);
            if (ret)
                return ret;
            vv->state = state_t::playlist;
        }
        else
        {
            if (vv->playing_playlist != max_plb_files)
                seek_pl_list(&vv->pll, vv->playing_playlist);
        }
        *need_redraw = 1;
        break;
        
    case state_t::playlist:
        if (playlist_check_near(&vv->plv, &vv->pl))
        {
            vv->state = state_t::song;
            *need_redraw = 1;
        }
        else
        {
            view_to_playing_playlist(vv, need_redraw);
        }
        break;
        
    case state_t::song:
        process_toggle_repeat(vv, need_redraw);
        break;
    }
    return 0;
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
    for (uint32_t i = 0; i != joystick_states_cnt; ++i)
    {
        if (check_button_state(buttons[i]))
        {
            uint32_t ret = process_view_do[i](vv, need_redraw);
            if (ret)
                return ret;
        }
    }
    return 0;
}

