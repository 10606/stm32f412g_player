#include "stm32f4xx_it.h"
#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"

extern char seeked;
void init_mad ();
void deinit_mad ();

uint32_t no_plb_files = 401;

uint32_t seek_value = (1024 * 32);

uint32_t init_view (view * vv, char (* path)[12], uint32_t len, audio_ctl * buffer_ctl)
{
    vv->buffer_ctl = buffer_ctl;
    vv->state = D_PL_LIST;
    vv->state_song_view = S_VOLUME;
    vv->playing_playlist = max_plb_files;
    vv->selected_playlist = max_plb_files;
    vv->pl.fd = &vv->fd_pl;
    bind_playlist_view(&vv->plv, &vv->fd_plv);
    
    //BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"bind plv", LEFT_MODE);
    uint32_t ret;
    ret = init_pl_list(&vv->pll, path, len);
    if (ret)
        return ret;
    //BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"init pl_list", LEFT_MODE);
    if (vv->pll.cnt == 0)
        return no_plb_files;
    
    ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist);
    if (ret)
        return ret;
    //BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"open pl_list", LEFT_MODE);
    
    ret = play(&vv->plv, &vv->pl);
    if (ret)
        return ret;
    //BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"play plv", LEFT_MODE);
    vv->playing_playlist = vv->selected_playlist;
    return 0;
}

uint32_t destroy_view (view * vv)
{
    destroy_pl_list(&vv->pll);
    destroy_playlist(&vv->pl);
    return 0;
}

void display_view (view * vv)
{
    switch (vv->state)
    {
    case D_PL_LIST:
        display_playlist(&vv->plv, &vv->pl, vv->state, 0);
        display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, 1);
        display_song(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 0);
        break;

    case D_PLAYLIST:
        display_playlist(&vv->plv, &vv->pl, vv->state, 1);
        display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, 0);
        display_song(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 0);
        break;
        
    case D_SONG:
        display_playlist(&vv->plv, &vv->pl, vv->state, 0);
        display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, 0);
        display_song(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 1);
        break;

    default:
        display_playlist(&vv->plv, &vv->pl, vv->state, 0);
        display_song(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 0);
        display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl, 0);
    }
}

uint32_t process_view_play_pause (view * vv, uint8_t * need_redraw)
{
    if (vv->buffer_ctl->pause_status == 1)
    { // Pause is enabled, call Resume
        BSP_AUDIO_OUT_Resume();
        vv->buffer_ctl->pause_status = 0;
    }
    else
    { // Pause the playback
        BSP_AUDIO_OUT_Pause();
        vv->buffer_ctl->pause_status = 1;
    }
    *need_redraw = 1;
    return 0;
}

uint32_t process_view_inc_volume (view * vv, uint8_t * need_redraw)
{
    vv->buffer_ctl->volume += 1;
    if (vv->buffer_ctl->volume > 100)
        vv->buffer_ctl->volume = 100;
    BSP_AUDIO_OUT_SetVolume(vv->buffer_ctl->volume);
    display_song_volume(&vv->pl, vv->buffer_ctl, &vv->state_song_view, (vv->state == D_SONG));
    return 0;
}

uint32_t process_view_seek_forward (view * vv, uint8_t * need_redraw)
{
    uint32_t ret, new_pos;
    new_pos = current_position(&vv->buffer_ctl->audio_file);
    if ((vv->buffer_ctl->audio_file.size < seek_value) ||
        (new_pos > vv->buffer_ctl->audio_file.size - seek_value))
        new_pos = vv->buffer_ctl->audio_file.size;
    else
        new_pos += seek_value;
    seeked = 1;
    ret = f_seek(&vv->buffer_ctl->audio_file, new_pos);
    if (ret)
    {
        return ret;
    }
    return 0;
}

uint32_t process_view_prev_song (view * vv, uint8_t * need_redraw)
{
    uint32_t ret;
    ret = prev_playlist(&vv->pl);
    if (ret)
    {
        return ret;
    }
    else
    {
        deinit_mad();
        init_mad();
        if ((ret = open_song(&vv->pl, &vv->buffer_ctl->audio_file)))
            return ret;
        seeked = 1;
        get_length(&vv->buffer_ctl->audio_file, &vv->buffer_ctl->info);
        if ((ret = f_seek(&vv->buffer_ctl->audio_file, vv->buffer_ctl->info.offset)))
            return ret;
        vv->buffer_ctl->audio_file.size = vv->buffer_ctl->audio_file.size;
    }
    *need_redraw = 1;
    return 0;
}

uint32_t process_view_up (view * vv, uint8_t * need_redraw)
{
    switch (vv->state)
    {
    case D_PL_LIST:
        up_pl_list(&vv->pll);
        *need_redraw = 1;
        break;

    case D_PLAYLIST:
        up(&vv->plv);
        *need_redraw = 1;
        break;
        
    case D_SONG:
        switch (vv->state_song_view)
        {
        case S_VOLUME:
            return process_view_inc_volume(vv, need_redraw);
            
        case S_SEEK:
            return process_view_seek_forward(vv, need_redraw);

        case S_NEXT_PREV:
            return process_view_prev_song(vv, need_redraw);
        }
        break;
    }
    return 0;
}

uint32_t process_view_dec_volume (view * vv, uint8_t * need_redraw)
{
    if (vv->buffer_ctl->volume < 1)
        vv->buffer_ctl->volume = 0;
    else
        vv->buffer_ctl->volume -= 1;
    BSP_AUDIO_OUT_SetVolume(vv->buffer_ctl->volume);
    display_song_volume(&vv->pl, vv->buffer_ctl, &vv->state_song_view, (vv->state == D_SONG));
    return 0;
}

uint32_t process_view_seek_backward (view * vv, uint8_t * need_redraw)
{
    uint32_t ret, new_pos;
    new_pos = current_position(&vv->buffer_ctl->audio_file);
    if (new_pos < seek_value)
        new_pos = 0;
    else
        new_pos -= seek_value;
    seeked = 1;
    ret = f_seek(&vv->buffer_ctl->audio_file, new_pos);
    if (ret)
    {
        return ret;
    }
    return 0;
}

uint32_t process_view_next_song (view * vv, uint8_t * need_redraw)
{
    uint32_t ret;
    ret = next_playlist(&vv->pl);
    if (ret)
    {
        return ret;
    }
    else
    {
        deinit_mad();
        init_mad();
        if ((ret = open_song(&vv->pl, &vv->buffer_ctl->audio_file)))
            return ret;
        seeked = 1;
        get_length(&vv->buffer_ctl->audio_file, &vv->buffer_ctl->info);
        if ((ret = f_seek(&vv->buffer_ctl->audio_file, vv->buffer_ctl->info.offset)))
            return ret;
        vv->buffer_ctl->audio_file.size = vv->buffer_ctl->audio_file.size;
    }
    *need_redraw = 1;
    return 0;
}

uint32_t process_view_down (view * vv, uint8_t * need_redraw)
{
    switch (vv->state)
    {
    case D_PL_LIST:
        down_pl_list(&vv->pll);
        *need_redraw = 1;
        break;

    case D_PLAYLIST:
        down(&vv->plv);
        *need_redraw = 1;
        break;
        
    case D_SONG:
        switch (vv->state_song_view)
        {
        case S_VOLUME:
            return process_view_dec_volume(vv, need_redraw);
            
        case S_SEEK:
            return process_view_seek_backward(vv, need_redraw);

        case S_NEXT_PREV:
            return process_view_next_song(vv, need_redraw);
        }
        break;
    }
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
        play(&vv->plv, &vv->pl);
        vv->playing_playlist = vv->selected_playlist;
        deinit_mad();
        init_mad();
        ret = open_song(&vv->pl, &vv->buffer_ctl->audio_file);
        if (ret)
            return ret;
        seeked = 1;
        get_length(&vv->buffer_ctl->audio_file, &vv->buffer_ctl->info);
        if ((ret = f_seek(&vv->buffer_ctl->audio_file, vv->buffer_ctl->info.offset)))
            return ret;
        vv->buffer_ctl->audio_file.size = vv->buffer_ctl->audio_file.size;
        *need_redraw = 1;
        break;
        
    case D_SONG:
        vv->state_song_view = (vv->state_song_view + 1) % state_song_view_cnt;
        display_song_volume(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 1);
        break;
    }
    return 0;
}

uint32_t process_view_center (view * vv, uint8_t * need_redraw)
{
    uint32_t ret;
    switch (vv->state)
    {
    case D_PL_LIST:
        seek_pl_list(&vv->pll, vv->playing_playlist);
        vv->state = D_PLAYLIST;
        ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist);
        if (ret)
            return ret;
        *need_redraw = 1;
        break;
        
    case D_PLAYLIST:
        if (check_near(&vv->plv, &vv->pl))
        {
            vv->state = D_SONG;
        }
        else
        {
            if (!compare(&vv->plv.lpl, &vv->pl))
            {
                init_playlist_view(&vv->plv, vv->pl.fd);
            }
            seek_playlist_view(&vv->plv, vv->pl.pos);
        }
        *need_redraw = 1;
        break;
        
    case D_SONG:
        vv->buffer_ctl->repeat_mode ^= 1;
        display_song_volume(&vv->pl, vv->buffer_ctl, &vv->state_song_view, 1);
        break;
    }
    return 0;
}

inline char check_button_state (uint32_t joy_button)
{
    char ans = 
        ((joystick_state.process[joy_button] >= 1) &&
         (joystick_state.prev_processed[joy_button] == 0)) ||
        ((joystick_state.process[joy_button] >= 4) &&
         (joystick_state.prev_processed[joy_button] == 1)) ||
        ((joystick_state.process[joy_button] >= 1) &&
         (joystick_state.prev_processed[joy_button] == 2));
    if (ans)
    {
        switch (joystick_state.prev_processed[joy_button])
        {
            case 0:
                joystick_state.process[joy_button] -= 1;
                break;
            case 1:
                joystick_state.process[joy_button] -= 4;
                break;
            case 2:
                joystick_state.process[joy_button] -= 1;
                break;
        }
        joystick_state.prev_processed[joy_button]++;
        if (joystick_state.prev_processed[joy_button] > 2)
            joystick_state.prev_processed[joy_button] = 2;
    }
    return ans;
}

uint32_t process_view (view * vv, uint8_t * need_redraw)
{
    char flag = 1;
    while (flag)
    {
        flag = 0;
        if (check_button_state(joy_button_up))
        {
            flag = 1;
            process_view_up(vv, need_redraw);
        }
        if (check_button_state(joy_button_down))
        {
            flag = 1;
            process_view_down(vv, need_redraw);
        }
        if (check_button_state(joy_button_left))
        {
            flag = 1;
            process_view_left(vv, need_redraw);
        }
        if (check_button_state(joy_button_right))
        {
            flag = 1;
            process_view_right(vv, need_redraw);
        }
        if (check_button_state(joy_button_center))
        {
            flag = 1;
            process_view_center(vv, need_redraw);
        }
    }
    return 0;
}

