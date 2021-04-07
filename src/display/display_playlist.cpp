#include "display.h"

#include "lcd_display.h"
#include "stm32f4xx_hal_gpio.h"
#include "audio.h"
#include "usb_send.h"

namespace display
{

void cur_song (playlist * pl_p, char to_screen, uint8_t * need_redraw)
{
    char cur_song_name[song_name_sz + 1];
    char cur_group_name[group_name_sz + 1];
    memcpy(cur_song_name, pl_p->song.song_name, song_name_sz);
    cur_song_name[song_name_sz] = 0;
    memcpy(cur_group_name, pl_p->song.group_name, group_name_sz);
    cur_group_name[group_name_sz] = 0;
    color_t yb = {lcd_color_yellow, lcd_color_blue};
    if (to_screen)
    {
        display_string(10, display::offsets::song_name, cur_group_name, &font_16, &yb);
        display_string(10, display::offsets::song_name + display::offsets::in_song_name, cur_song_name, &font_16, &yb);
        audio_process(need_redraw);
    }
    send_cur_song(cur_group_name, cur_song_name);
}

void cur_playlist (playlist_view * plv, playlist * pl_p, char to_screen, uint8_t * need_redraw)
{
    uint32_t y_pos = display::offsets::list + display::offsets::line * playlist_view_cnt;
    if (to_screen)
        fill_rect(0, y_pos, 240, 240 - y_pos, lcd_color_white);
    cur_song(pl_p, to_screen, need_redraw);
    HAL_Delay(1);
    audio_process(need_redraw);
    
    char song_name[playlist_view_cnt][song_name_sz + 1];
    char group_name[playlist_view_cnt][group_name_sz + 1];
    char selected[playlist_view_cnt];
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
    char number[playlist_view_cnt][3 + 1];
    
    plv->print(pl_p, song_name, group_name, selected, number);
    
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
    
        uint16_t back_color_group, text_color_group;
        uint16_t back_color_song, text_color_song;
        
        switch (selected[i])
        {
            case 3:
                back_color_group = lcd_color_blue;
                text_color_group = lcd_color_yellow;
                back_color_song = lcd_color_blue;
                text_color_song = lcd_color_green;
                break;
            case 2:
                back_color_song = back_color_group = lcd_color_white;
                text_color_song = text_color_group = lcd_color_red;
                break;
            case 1:
                back_color_song = back_color_group = lcd_color_blue;
                text_color_song = text_color_group = lcd_color_white;
                break;
            default:
                back_color_song = back_color_group = lcd_color_white;
                text_color_song = text_color_group = lcd_color_blue;
        }

        color_t c_group = {text_color_group, back_color_group};
        color_t c_song = {text_color_song, back_color_song};
        if (to_screen)
        {
            display_string(4, display::offsets::list + display::offsets::line * i, s_group, &font_12, &c_group);
            display_string(4, display::offsets::list + display::offsets::in_line + display::offsets::line * i, s_song, &font_12, &c_song);
        }
        audio_process(need_redraw);
        send_displayed_song(s_group, s_song, selected[i], i);
    }
}

}

