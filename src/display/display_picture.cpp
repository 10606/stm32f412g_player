#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "usb_send.h"

namespace display
{

picture_info_t picture_info;
const uint16_t * const picture_info_t::song = reinterpret_cast <uint16_t const *> (0x08080000);
const uint16_t * const picture_info_t::err = reinterpret_cast <uint16_t const *> (0x080c0000);
 

void start_image ()
{
    scroller.reset();
    picture_info.update_start_pic_num();
    draw_RGB_image(0, 0, 240, 240, picture_info.start_pic());
    
    {
        decltype(volume_info_t::line_0) s_volume;
        decltype(volume_info_t::line_1) s_state;
        memset(s_volume, ' ', sizeof(s_volume));
        memset(s_state, ' ', sizeof(s_state));
        HAL_Delay(1);
        send_volume(s_volume, s_state);
    }
    {
        decltype(cur_song_info_t::line_0) cur_group_name;
        decltype(cur_song_info_t::line_1) cur_song_name;
        memset(cur_group_name, ' ', sizeof(cur_group_name));
        memset(cur_song_name, ' ', sizeof(cur_song_name));
        HAL_Delay(1);
        send_cur_song(cur_group_name, cur_song_name);
    }
    {
        char selected[playlist_view_cnt];
        memset(selected, 0, sizeof(selected));
        
        decltype(displayed_song_info_t::line_0) s_group;
        decltype(displayed_song_info_t::line_1) s_song;
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
        decltype(pl_list_info_t::name) s_playlist;
        memset(s_playlist, ' ', sizeof(s_playlist));
        
        for (uint32_t i = 0; i != plb_view_cnt; ++i)
        {
            HAL_Delay(1);
            send_pl_list(s_playlist, selected[i], i);
        }
    }
}

void song_image (bool & need_redraw)
{
    scroller.reset();
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

}

