#include "display.h"

#include "audio.h"
#include "pl_list.h"
#include "display_string.h"
#include "usb_send.h"
#include <stdint.h>

void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p, char to_screen, uint8_t * need_redraw)
{
    uint32_t y_pos = list_offset + line_offset * plb_view_cnt;
    if (to_screen)
        fill_rect(0, y_pos, 240, 240 - y_pos, lcd_color_white);
    display_cur_song(pl_p, to_screen, need_redraw);
    
    char playlist_name[plb_view_cnt][pl_name_sz + 1];
    char number[plb_view_cnt][3 + 1];
    char count[plb_view_cnt][3 + 1];
    char selected[plb_view_cnt];
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
 
    char empty[name_offset + pl_name_sz + count_offset + 3 + 1];
    memset(empty, ' ', sizeof(empty));
    empty[count_offset + 3] = 0;
    
    print_pl_list(pll, playing_pl, playlist_name, number, count, selected);
    
    for (uint32_t i = 0; i != plb_view_cnt; ++i)
    {
        char s_playlist[name_offset + pl_name_sz + count_offset + 3 + 1];
        memset(s_playlist, ' ', sizeof(s_playlist));
        memcpy(s_playlist, number[i], 3);
        memcpy(s_playlist + name_offset, playlist_name[i], pl_name_sz);
        memcpy(s_playlist + count_offset, count[i], 3);
        s_playlist[count_offset + 3] = 0;

        uint32_t back_color_group, text_color_group;
        uint32_t back_color_song, text_color_song;
        
        switch (selected[i])
        {
            case 3:
                back_color_group = lcd_color_blue;
                text_color_group = lcd_color_green;
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
            display_string(4, list_offset + line_offset * i, s_playlist, &font_12, &c_group);
            display_string(4, list_offset + in_line_offset + line_offset * i, empty, &font_12, &c_song);
        }
        audio_process(need_redraw);
        send_pl_list(s_playlist, selected[i], i);
    }
}

