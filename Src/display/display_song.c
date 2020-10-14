#include "display.h"

#include "st7789h2/st7789h2.h"
#include "display_string.h"

inline uint32_t min (uint32_t a, uint32_t b)
{
    return (a < b)? a : b;
}

void display_err ()
{
    ST7789H2_DrawRGBImage(0, 0, 240, 240, err_picture_address);
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
    
    color_t bw = {LCD_COLOR_BLUE, LCD_COLOR_WHITE};
    display_string(4, list_offset, (uint8_t *)s_volume, &Font12, &bw);
    AUDIO_Process();
}

void display_picture ()
{
    uint32_t parts = 5;
    uint32_t p_size = (240 - picture_offset + parts - 1) / parts;
    uint32_t p_old_size = p_size;
    
    for (uint32_t part = 0; part != parts; ++part)
    {
        if (part + 1 == parts)
            p_size = (240 - picture_offset) - (parts - 1) * p_old_size;
        ST7789H2_DrawRGBImage(0, picture_offset + p_old_size * part, 240, p_size, song_picture_address + 2 * 240 * part * p_old_size);
        AUDIO_Process();
    }
}

void display_song (playlist * pl, audio_ctl * actl, state_song_view_t * state) //TODO
{
    display_cur_song(pl);
    
    //BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    //BSP_LCD_FillRect(0, HEADBAND_HEIGHT, BSP_LCD_GetXSize(), BSP_LCD_GetYSize() - HEADBAND_HEIGHT);
    AUDIO_Process();
    
    display_song_volume(pl, actl, state);

    char s_freq[15];
    snprintf(s_freq, sizeof(s_freq), "     %luhz", *actl->audio_freq_ptr);
    
    color_t bw = {LCD_COLOR_BLUE, LCD_COLOR_WHITE};
    display_string(4, list_offset + in_line_offset, (uint8_t *)s_freq, &Font12, &bw);
    AUDIO_Process();
    
    /*
    for (uint32_t i = 0; i != min(name_limit, pl->path_sz); ++i)
    {
        display_string(4, list_offset + line_offset + in_line_offset * (i + 1), (uint8_t *)pl->path[i], &Font12, &bw);
        AUDIO_Process();
    }
    */
 
    display_picture();
}

