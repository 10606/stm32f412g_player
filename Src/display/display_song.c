#include "display.h"

#include "st7789h2/st7789h2.h"
#include "display_string.h"
#include "usb_send.h"

inline uint32_t min (uint32_t a, uint32_t b)
{
    return (a < b)? a : b;
}

void display_err ()
{
    ST7789H2_DrawRGBImage(0, 0, 240, 240, err_picture_address);
}

void display_song_volume (playlist * pl, audio_ctl * actl, state_song_view_t * state, char to_screen) //TODO
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
    
    char s_volume[volume_width];
    char s_state[volume_width];
    color_t bw = {LCD_COLOR_BLUE, LCD_COLOR_WHITE};
    snprintf(s_volume, sizeof(s_volume), " %3lu%%", (actl->volume % 101));
    snprintf(s_state, sizeof(s_state), "  %c %c", (actl->repeat_mode? 'r' : ' '), c_state);
    if (to_screen)
    {
        display_string(200, list_offset, (uint8_t *)s_volume, &Font12, &bw);
        display_string(200, list_offset + in_line_offset, (uint8_t *)s_state, &Font12, &bw);
        AUDIO_Process();
    }
    HAL_Delay(1);
    send_volume(s_volume, s_state);
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

void display_song (playlist * pl, audio_ctl * actl, state_song_view_t * state, char to_screen) //TODO
{
    if (to_screen)
    {
        display_picture();
        AUDIO_Process();
    }
    display_cur_song(pl, to_screen);
    display_song_volume(pl, actl, state, to_screen);
    AUDIO_Process();
}

