#include "display.h"

#include "lcd_display.h"
#include <stdio.h>
#include "audio.h"
#include "usb_send.h"

void display_song_hint ()
{
    fill_rect(0, headband_height, 240, 240 - headband_height, lcd_color_white);
    fill_rect(0, 0, 240, headband_height, lcd_color_blue);
}

void display_start_image ()
{
    draw_RGB_image(0, 0, 240, 240, err_picture_address);
}

void display_song_volume (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, uint8_t * need_redraw) 
{
    char c_state = ' ';
    switch (state)
    {
    case state_song_view_t::volume:
        c_state = 'v';
        break;
        
    case state_song_view_t::seek:
        c_state = 's';
        break;
    
    case state_song_view_t::next_prev:
        c_state = 'n';
        break;
    }
    
    char s_volume[volume_width];
    char s_state[volume_width];
    memset(s_volume, ' ', sizeof(s_volume));
    memset(s_state, ' ', sizeof(s_state));
    color_t bw = {lcd_color_blue, lcd_color_white};
    snprintf(s_volume, sizeof(s_volume), " %3u%%", actl->volume);
    snprintf(s_state, sizeof(s_state), "  %c %c", (actl->repeat_mode? 'r' : ' '), c_state);
    if (to_screen)
    {
        display_string(200, list_offset, s_volume, &font_12, &bw);
        display_string(200, list_offset + in_line_offset, s_state, &font_12, &bw);
        audio_process(need_redraw);
    }
    HAL_Delay(1);
    send_volume(s_volume, s_state);
}

void display_picture (uint8_t * need_redraw)
{
    uint32_t parts = 5;
    uint32_t p_size = (240 - picture_offset + parts - 1) / parts;
    uint32_t p_old_size = p_size;
    
    for (uint32_t part = 0; part != parts; ++part)
    {
        if (part + 1 == parts)
            p_size = (240 - picture_offset) - (parts - 1) * p_old_size;
        draw_RGB_image(0, picture_offset + p_old_size * part, 240, p_size, song_picture_address + 240 * part * p_old_size);
        audio_process(need_redraw);
    }
}

void display_song (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, char redraw_picture, uint8_t * need_redraw) 
{
    if (to_screen && redraw_picture) // don't redraw picture if not need
    {
        display_picture(need_redraw);
        audio_process(need_redraw);
    }
    display_cur_song(pl, to_screen, need_redraw);
    display_song_volume(pl, actl, state, to_screen, need_redraw);
    audio_process(need_redraw);
}

