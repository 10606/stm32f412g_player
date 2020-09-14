#include "stm32f4xx_it.h"
#include "view.h"

uint32_t no_plb_files = 401;

uint32_t init_view (view * vv, char (* path)[12], uint32_t len, audio_ctl * buffer_ctl)
{
    vv->buffer_ctl = buffer_ctl;
    vv->state = D_PL_LIST;
    vv->playing_playlist = max_plb_files;
    vv->selected_playlist = max_plb_files;
    vv->pl.fd = &vv->fd_pl;
    bind_playlist_view(&vv->plv, &vv->fd_plv);
    
    uint32_t ret;
    if ((ret = init_pl_list(&vv->pll, path, len)))
        return ret;
    if ((vv->pll.cnt == 0))
        return no_plb_files;
    
    if ((ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist)))
        return ret;
    
    play(&vv->plv, &vv->pl);
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
            display_pl_list(&vv->pll, vv->playing_playlist, &vv->pl);
            break;
        case D_PLAYLIST:
            display_playlist(&vv->plv, &vv->pl);
            break;
    }
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
    }
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
        if ((ret = open_selected_pl_list(&vv->pll, &vv->plv, &vv->selected_playlist)))
        {
            return ret;
            //BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
        }
        *need_redraw = 1;
        break;
    case D_PLAYLIST:
        play(&vv->plv, &vv->pl);
        vv->playing_playlist = vv->selected_playlist;
        if ((ret = open_song(&vv->pl, &vv->buffer_ctl->audio_file)))
        {
            return ret;
            //BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
        }
        vv->buffer_ctl->audio_file_size = vv->buffer_ctl->audio_file.size;
        vv->buffer_ctl->fptr = 0;
        *need_redraw = 1;
        break;
    }
    return 0;
}

uint32_t process_view (view * vv, uint8_t * need_redraw)
{
    if (joystick_state.process[joy_button_up] > 1)
    {
        joystick_state.process[joy_button_up] = 0;
        process_view_up(vv, need_redraw);
    }
    if (joystick_state.process[joy_button_down] > 1)
    {
        joystick_state.process[joy_button_down] = 0;
        process_view_down(vv, need_redraw);
    }
    if (joystick_state.process[joy_button_left] > 1)
    {
        joystick_state.process[joy_button_left] = 0;
        process_view_left(vv, need_redraw);
    }
    if (joystick_state.process[joy_button_right] > 1)
    {
        joystick_state.process[joy_button_right] = 0;
        process_view_right(vv, need_redraw);
    }
    return 0;
}

