#include "display.h"

#include "display_common.h"
#include "lcd_display.h"
#include "stm32f4xx_hal_gpio.h"
#include "audio.h"
#include "usb_send.h"

namespace display
{

void cur_playlist 
(
    playlist_view & plv, 
    playlist const & pl, 
    bool to_screen, 
    bool redraw_screen,  
    bool & need_redraw
)
{
    if (to_screen)
        fill_borders <playlist_view_cnt> ();
    HAL_Delay(1);
    audio_ctl.audio_process(need_redraw);
    
    char song_name[playlist_view_cnt][song_name_sz + 1];
    char group_name[playlist_view_cnt][group_name_sz + 1];
    char selected[playlist_view_cnt];
    char number[playlist_view_cnt][3 + 1];
    
    plv.print(pl, song_name, group_name, selected, number);
    redraw_type_t redraw_type = plv.redraw_type();
    plv.reset_display();

    if (to_screen)
        scroll_text <playlist_view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = playlist_view_cnt;
    
    uint32_t pos_playing = playlist_view_cnt;
    for (uint32_t i = 0; i != playlist_view_cnt; ++i)
    {
        char s_group[name_offset + group_name_sz + 1];
        memset(s_group, ' ', name_offset + 1);
        memcpy(s_group, number[i], 3);
        memcpy(s_group + name_offset, group_name[i], group_name_sz);
        s_group[name_offset + group_name_sz] = 0;

        char s_song[name_offset + song_name_sz + 1];
        memset(s_song, ' ', name_offset + 1);
        memcpy(s_song + name_offset, song_name[i], song_name_sz);
        s_song[name_offset + song_name_sz] = 0;
    
        if (to_screen)
            display_lines(i, old_pos_playing, s_song, s_group, selected, redraw_type, playlist_view_cnt, lcd_color_yellow, redraw_screen);
        if (selected[i] & 2)
            pos_playing = i; 
        
        //audio_ctl.audio_process(need_redraw);
        send_displayed_song(s_group, s_song, selected[i], i);
    }
    old_pos_playing = pos_playing;
}

void display_lines 
(
    uint32_t i, 
    uint32_t old_pos_playing,
    char * line_0, 
    char * line_1, 
    char * selected, 
    redraw_type_t const & redraw_type, 
    uint32_t view_cnt, 
    uint16_t l0_text_color,
    bool force_redraw
)
{
    uint32_t old_pos = (redraw_type.pos + view_cnt + (redraw_type.direction? 1 : -1)) % view_cnt;
    uint32_t border = redraw_type.direction? 0 : (view_cnt - 1);
    
    if (//(redraw_type.type != redraw_type_t::nothing) &&
        (
            force_redraw ||
            
            (i == old_pos_playing) || // playing
            (selected[i] & 2) ||
            
            (redraw_type.type == redraw_type_t::not_easy) ||
            
            (i == redraw_type.pos) ||
            (i == old_pos) ||
            ((i == border) && 
             (redraw_type.type == redraw_type_t::middle))
        )
    )
    {
        uint16_t back_color_line_0, text_color_line_0;
        uint16_t back_color_line_1, text_color_line_1;
        
        switch (selected[i])
        {
            // 0
            // 1 - selected
            // 2 - playing
            // 3 - playing and selected
            case 3:
                back_color_line_0 = lcd_color_blue;
                text_color_line_0 = l0_text_color;
                back_color_line_1 = lcd_color_blue;
                text_color_line_1 = lcd_color_green;
                break;
            case 2:
                back_color_line_1 = back_color_line_0 = lcd_color_white;
                text_color_line_1 = text_color_line_0 = lcd_color_red;
                break;
            case 1:
                back_color_line_1 = back_color_line_0 = lcd_color_blue;
                text_color_line_1 = text_color_line_0 = lcd_color_white;
                break;
            default:
                back_color_line_1 = back_color_line_0 = lcd_color_white;
                text_color_line_1 = text_color_line_0 = lcd_color_blue;
        }

        color_t c_line_0 = {text_color_line_0, back_color_line_0};
        color_t c_line_1 = {text_color_line_1, back_color_line_1};
        
        display_string(4, scroller.recalc_y(display::offsets::list + display::offsets::line * i),
                    line_0, &font_12, &c_line_0);
        display_string(4, scroller.recalc_y(display::offsets::list + display::offsets::line * i + display::offsets::in_line), 
                    line_1, &font_12, &c_line_1);
    }
}

}

