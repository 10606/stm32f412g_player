#include "display_song.h"

#include "display_picture.h"
#include "display_common.h"
#include "lcd_display.h"
#include <stdio.h>
#include "audio.h"
#include "usb_send.h"

namespace display
{

void cur_song (playlist const & pl)
{
    char cur_song_name[sz::song_name + 1];
    char cur_group_name[sz::group_name + 1];
    memcpy(cur_song_name, pl.lpl.song.song_name, sizeof(pl.lpl.song.song_name));
    cur_song_name[sz::song_name] = 0;
    memcpy(cur_group_name, pl.lpl.song.group_name, sizeof(pl.lpl.song.group_name));
    cur_group_name[sz::group_name] = 0;
    color_t yb = {lcd_color_yellow, lcd_color_blue};
    
    display_string(10, display::offsets::song_name, cur_group_name, &font_16, &yb);
    display_string(10, display::offsets::song_name + display::offsets::in_song_name, cur_song_name, &font_16, &yb);
    audio_ctl.audio_process();
    
    send_cur_song(cur_group_name, cur_song_name);
}

void song_hint ()
{
    fill_rect(0, display::offsets::headband, 240, 240 - display::offsets::headband, lcd_color_white);
    fill_rect(0, 0, 240, display::offsets::headband, lcd_color_blue);
}

void song_volume 
(
    audio_ctl_t const & actl, 
    state_song_view_t state, 
    bool to_screen
) 
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
    
    char p_state = ' ';
    switch (actl.pause_status)
    {
    case 0: [[likely]]
        break;
    case 1:
        p_state = 7;
        break;
    case 2:
        p_state = 6;
        break;
    }
    
    char s_volume[sz::volume];
    char s_state[sz::volume];
    memset(s_volume, ' ', sizeof(s_volume));
    memset(s_state, ' ', sizeof(s_state));
    snprintf(s_volume, sizeof(s_volume), " %3u%%", actl.volume);
    snprintf(s_state, sizeof(s_state), "  %c%c%c", (actl.repeat_mode? 'r' : ' '), p_state, c_state);
    
    if (to_screen)
    {
        // adapvite background color 
        display_string_c(208, display::offsets::list - 2, 
                         s_volume + 1, &font_12, 
                         picture_info.color, lcd_color_blue);
        display_string_c(208, display::offsets::list - 2+ display::offsets::in_line, 
                         s_state + 1, &font_12, 
                         picture_info.color, lcd_color_blue);
        audio_ctl.audio_process();
    }
    HAL_Delay(1);
    send_volume(s_volume, s_state);
}

void song 
(
    audio_ctl_t const & actl, 
    state_song_view_t state, 
    state_t cur_state, 
    state_t old_state
) 
{
    bool to_screen = cur_state == state_t::song;
    if (to_screen && old_state != state_t::song) // don't redraw picture if not need
        song_image();
    song_volume(actl, state, to_screen);
    audio_ctl.audio_process();
}

}

