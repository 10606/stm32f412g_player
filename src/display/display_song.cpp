#include "display.h"

#include "lcd_display.h"
#include <stdio.h>
#include "audio.h"
#include "usb_send.h"

namespace display
{

struct picture_info_t 
{
    picture_info_t () :
        song_picture_number(0),
        start_picture_number(0)
    {}
    
    void update_song_pic_num ()
    {
        song_picture_number = (song_picture_number + 1) % cnt_pictures_song;
    }
    
    void update_start_pic_num ()
    {
        start_picture_number = (start_picture_number + 1) % cnt_pictures_start;
    }
    
    const uint16_t * song_pic ()
    {
        return song + song_picture_number * offset;
    }
    
    const uint16_t * start_pic ()
    {
        return err + start_picture_number * offset;
    }

private:
    uint8_t song_picture_number;
    uint8_t start_picture_number;
    
    static const uint16_t * const song;
    static const uint16_t * const err;
    
    static const size_t offset = 0x10000; // in uint16_t
    static const uint8_t cnt_pictures_song = 2;
    static const uint8_t cnt_pictures_start = 2;

} picture_info;

const uint16_t * const picture_info_t::song = reinterpret_cast <uint16_t const *> (0x08080000);
const uint16_t * const picture_info_t::err = reinterpret_cast <uint16_t const *> (0x080c0000);
 

void song_hint ()
{
    fill_rect(0, display::offsets::headband, 240, 240 - display::offsets::headband, lcd_color_white);
    fill_rect(0, 0, 240, display::offsets::headband, lcd_color_blue);
}

void start_image ()
{
    picture_info.update_start_pic_num();
    draw_RGB_image(0, 0, 240, 240, picture_info.start_pic());
    
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

void display_picture (bool & need_redraw)
{
    picture_info.update_song_pic_num();
    static constexpr uint32_t parts = 5;
    uint32_t p_size = (240 - display::offsets::picture + parts - 1) / parts;
    uint32_t p_old_size = p_size;
    
    uint16_t const * picture = picture_info.song_pic(); 
    for (uint32_t part = 0; part != parts; ++part)
    {
        if (part + 1 == parts)
            p_size = (240 - display::offsets::picture) - (parts - 1) * p_old_size;
        draw_RGB_image(0, display::offsets::picture + p_old_size * part, 
                       240, p_size, 
                       picture + 240 * part * p_old_size);
        audio_ctl.audio_process(need_redraw);
    }
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
        display_picture(need_redraw);
    song_volume(actl, state, to_screen, need_redraw);
    audio_ctl.audio_process(need_redraw);
}

}

