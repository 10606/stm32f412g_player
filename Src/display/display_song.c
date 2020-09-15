#include "display.h"

#define in_line_offset 11
#define line_offset 23
#define list_offset 75
#define name_limit 11

inline uint32_t min (uint32_t a, uint32_t b)
{
    return (a < b)? a : b;
}

void display_song_volume (playlist * pl, audio_ctl * actl, state_song_view_t * state) //TODO
{
    char c_state = ' ';
    switch (*state)
    {
    case S_VOLUME:
        c_state = 'v';
        break;
        
    case S_SEEK:
        c_state = 's';
        break;
    
    case S_NEXT_PREV:
        c_state = 'n';
        break;
    }
    
    char s_volume[25];
    snprintf(s_volume, sizeof(s_volume), " volume %3lu%%    %c %c", (actl->volume % 101), (actl->repeat_mode? 'r' : ' '), c_state);
    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAt(4, list_offset, (uint8_t *)s_volume, LEFT_MODE);
    AUDIO_Process();
}

void display_song (playlist * pl, audio_ctl * actl, state_song_view_t * state) //TODO
{
    display_cur_song(pl);
    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, HEADBAND_HEIGHT, BSP_LCD_GetXSize(), BSP_LCD_GetYSize() - HEADBAND_HEIGHT);
    AUDIO_Process();
    
    display_song_volume(pl, actl, state);

    char s_freq[15];
    snprintf(s_freq, sizeof(s_freq), "     %luhz", *actl->audio_freq_ptr);
    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAt(4, list_offset + in_line_offset, (uint8_t *)s_freq, LEFT_MODE);
    AUDIO_Process();
    
    for (uint32_t i = 0; i != min(name_limit, pl->path_sz); ++i)
    {
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_DisplayStringAt(4, list_offset + line_offset + in_line_offset * (i + 1), (uint8_t *)pl->path[i], LEFT_MODE);
        AUDIO_Process();
    }
}

