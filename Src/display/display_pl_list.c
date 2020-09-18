#include "display.h"

#include "pl_list.h"
#include <stdint.h>

void display_pl_list (pl_list * pll, uint32_t playing_pl, playlist * pl_p)
{
    display_cur_song(pl_p);
    
    char playlist_name[max_plb_files][pl_name_sz + 1];
    char number[max_plb_files][3 + 1];
    char count[max_plb_files][3 + 1];
    char selected[max_plb_files];
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
 
    char empty[name_offset + pl_name_sz + count_offset + 3 + 1];
    memset(empty, ' ', sizeof(empty));
    empty[count_offset + 3] = 0;
    
    print_pl_list(pll, playing_pl, playlist_name, number, count, selected);
    
    for (uint32_t i = 0; i != view_plb_cnt; ++i)
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
                back_color_group = LCD_COLOR_BLUE;
                text_color_group = LCD_COLOR_GREEN;
                back_color_song = LCD_COLOR_BLUE;
                text_color_song = LCD_COLOR_GREEN;
                break;
            case 2:
                back_color_song = back_color_group = LCD_COLOR_WHITE;
                text_color_song = text_color_group = LCD_COLOR_RED;
                break;
            case 1:
                back_color_song = back_color_group = LCD_COLOR_BLUE;
                text_color_song = text_color_group = LCD_COLOR_WHITE;
                break;
            default:
                back_color_song = back_color_group = LCD_COLOR_WHITE;
                text_color_song = text_color_group = LCD_COLOR_BLUE;
        }

        BSP_LCD_SetBackColor(back_color_group);
        BSP_LCD_SetTextColor(text_color_group);
        BSP_LCD_DisplayStringAt(4, list_offset + line_offset * i, (uint8_t *)s_playlist, LEFT_MODE);
        AUDIO_Process();
        BSP_LCD_SetBackColor(back_color_song);
        BSP_LCD_SetTextColor(text_color_song);
        BSP_LCD_DisplayStringAt(4, list_offset + line_offset * i + in_line_offset, (uint8_t *)empty, LEFT_MODE);
        AUDIO_Process();
    }
}
