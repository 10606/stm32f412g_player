#include "display.h"

#define in_line_offset 11
#define line_offset 23
#define list_offset 75
#define name_offset 5

void display_cur_song (playlist * pl_p)
{
    char cur_song_name[song_name_sz + 1];
    char cur_group_name[group_name_sz + 1];
    memcpy(cur_song_name, pl_p->song.song_name, song_name_sz);
    cur_song_name[song_name_sz] = 0;
    memcpy(cur_group_name, pl_p->song.group_name, group_name_sz);
    cur_group_name[group_name_sz] = 0;
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(5, 20, (uint8_t *)cur_group_name, LEFT_MODE);
    AUDIO_Process();
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(5, 40, (uint8_t *)cur_song_name, LEFT_MODE);
    AUDIO_Process();
    BSP_LCD_SetFont(&Font12);
}

void display_playlist (playlist_view * plv, playlist * pl_p)
{
    display_cur_song(pl_p);
    
    char song_name[view_cnt][song_name_sz + 1];
    char group_name[view_cnt][group_name_sz + 1];
    char selected[view_cnt];
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
    char number[view_cnt][3 + 1];
    
    print_playlist_view(plv, pl_p, song_name, group_name, selected, number);
    
    for (uint32_t i = 0; i != view_cnt; ++i)
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
    
        uint32_t back_color_group, text_color_group;
        uint32_t back_color_song, text_color_song;
        
        switch (selected[i])
        {
            case 3:
                back_color_group = LCD_COLOR_BLUE;
                text_color_group = LCD_COLOR_YELLOW;
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
        BSP_LCD_DisplayStringAt(4, list_offset + line_offset * i, (uint8_t *)s_group, LEFT_MODE);
        AUDIO_Process();
        BSP_LCD_SetBackColor(back_color_song);
        BSP_LCD_SetTextColor(text_color_song);
        BSP_LCD_DisplayStringAt(4, list_offset + in_line_offset + line_offset * i, (uint8_t *)s_song, LEFT_MODE);
        AUDIO_Process();
    }
}

