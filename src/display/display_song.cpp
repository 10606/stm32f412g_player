#include "display.h"

#include "lcd_display.h"
#include <stdio.h>
#include "audio.h"
#include "usb_send.h"

namespace display
{

struct picture_address
{
    static const uint16_t * const song;
    static const uint16_t * const err;
};

const uint16_t * const picture_address::song = reinterpret_cast <uint16_t const *> (0x08080000);
const uint16_t * const picture_address::err = reinterpret_cast <uint16_t const *> (0x080c0000);
 

void song_hint ()
{
    fill_rect(0, display::offsets::headband, 240, 240 - display::offsets::headband, lcd_color_white);
    fill_rect(0, 0, 240, display::offsets::headband, lcd_color_blue);
}

void start_image ()
{
    draw_RGB_image(0, 0, 240, 240, picture_address::err);
    
    {
        char s_volume[volume_width];
        char s_state[volume_width];
        memset(s_volume, ' ', sizeof(s_volume));
        memset(s_state, ' ', sizeof(s_state));
        HAL_Delay(1);
        send_volume(s_volume, s_state);
    }
    {
        char cur_song_name[song_name_sz + 1];
        char cur_group_name[group_name_sz + 1];
        memset(cur_song_name, ' ', sizeof(cur_song_name));
        memset(cur_group_name, ' ', sizeof(cur_group_name));
        HAL_Delay(1);
        send_cur_song(cur_group_name, cur_song_name);
    }
    {
        char selected[playlist_view_cnt];
        memset(selected, 0, sizeof(selected));
        
        char s_group[name_offset + group_name_sz + 1];
        char s_song[name_offset + song_name_sz + 1];
        memset(s_group, ' ', sizeof(s_group));
        memset(s_song, ' ', sizeof(s_song));
    
        for (uint32_t i = 0; i != playlist_view_cnt; ++i)
        {
            HAL_Delay(1);
            send_displayed_song(s_group, s_song, selected[i], i);
        }
    }
    {
        char selected[plb_view_cnt];
        memset(selected, 0, sizeof(selected));
        char s_playlist[name_offset + pl_name_sz + count_offset + 3 + 1];
        memset(s_playlist, ' ', sizeof(s_playlist));
        
        for (uint32_t i = 0; i != plb_view_cnt; ++i)
        {
            HAL_Delay(1);
            send_pl_list(s_playlist, selected[i], i);
        }
    }
}

void song_volume (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, uint8_t * need_redraw) 
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
        display_string(200, display::offsets::list, s_volume, &font_12, &bw);
        display_string(200, display::offsets::list + display::offsets::in_line, s_state, &font_12, &bw);
        audio_ctl.audio_process(need_redraw);
    }
    HAL_Delay(1);
    send_volume(s_volume, s_state);
}

void display_picture (uint8_t * need_redraw)
{
    uint32_t parts = 5;
    uint32_t p_size = (240 - display::offsets::picture + parts - 1) / parts;
    uint32_t p_old_size = p_size;
    
    for (uint32_t part = 0; part != parts; ++part)
    {
        if (part + 1 == parts)
            p_size = (240 - display::offsets::picture) - (parts - 1) * p_old_size;
        draw_RGB_image(0, display::offsets::picture + p_old_size * part, 240, p_size, picture_address::song + 240 * part * p_old_size);
        audio_ctl.audio_process(need_redraw);
    }
}

void song (playlist * pl, audio_ctl_t * actl, state_song_view_t state, char to_screen, char redraw_picture, uint8_t * need_redraw) 
{
    if (to_screen && redraw_picture) // don't redraw picture if not need
    {
        display_picture(need_redraw);
        audio_ctl.audio_process(need_redraw);
    }
    cur_song(pl, to_screen, need_redraw);
    song_volume(pl, actl, state, to_screen, need_redraw);
    audio_ctl.audio_process(need_redraw);
}

}

