#include "display_song.h"

#include "display_picture.h"
#include "display_common.h"
#include "lcd_display.h"
#include <stdio.h>
#include "audio.h"
#include "usb_send.h"

namespace display
{

void cur_song (playlist const & pl, bool & need_redraw)
{
    char cur_song_name[song_name_sz + 1];
    char cur_group_name[group_name_sz + 1];
    memcpy(cur_song_name, pl.lpl.song.song_name, song_name_sz);
    cur_song_name[song_name_sz] = 0;
    memcpy(cur_group_name, pl.lpl.song.group_name, group_name_sz);
    cur_group_name[group_name_sz] = 0;
    color_t yb = {lcd_color_yellow, lcd_color_blue};
    
    display_string(10, display::offsets::song_name, cur_group_name, &font_16, &yb);
    display_string(10, display::offsets::song_name + display::offsets::in_song_name, cur_song_name, &font_16, &yb);
    audio_ctl.audio_process(need_redraw);
    
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
    bool to_screen, 
    bool & need_redraw
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
    
    char s_volume[volume_width];
    char s_state[volume_width];
    memset(s_volume, ' ', sizeof(s_volume));
    memset(s_state, ' ', sizeof(s_state));
    snprintf(s_volume, sizeof(s_volume), " %3u%%", actl.volume);
    snprintf(s_state, sizeof(s_state), "  %c %c", (actl.repeat_mode? 'r' : ' '), c_state);
    if (to_screen)
    {
        auto calc_offset = [] (uint32_t x, uint32_t y) -> uint32_t { return 240 * (display::offsets::list - display::offsets::picture + y) + x; };
        uint16_t const * picture = picture_info.song_pic(); 
        display_string_c(200, display::offsets::list, 
                         s_volume, &font_12, 
                         picture[calc_offset(199, 6)], lcd_color_blue);
        display_string_c(200, display::offsets::list + display::offsets::in_line, 
                         s_state, &font_12, 
                         picture[calc_offset(199, 2 * display::offsets::in_line)], lcd_color_blue);
        audio_ctl.audio_process(need_redraw);
    }
    HAL_Delay(1);
    send_volume(s_volume, s_state);
}

void song 
(
    audio_ctl_t const & actl, 
    state_song_view_t state, 
    bool to_screen, 
    bool redraw_picture, 
    bool & need_redraw
) 
{
    if (to_screen && redraw_picture) // don't redraw picture if not need
        song_image(need_redraw);
    song_volume(actl, state, to_screen, need_redraw);
    audio_ctl.audio_process(need_redraw);
}

}

