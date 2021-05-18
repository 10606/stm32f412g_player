#include "display_playlists.h"

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
    
    char song_name[playlist_view_cnt][sz::song_name + 1];
    char group_name[playlist_view_cnt][sz::group_name + 1];
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
        char s_group[sz::number + sz::group_name + 1];
        memset(s_group, ' ', sz::number + 1);
        memcpy(s_group, number[i], 3);
        memcpy(s_group + sz::number, group_name[i], sz::group_name);
        s_group[sz::number + sz::group_name] = 0;

        char s_song[sz::number + sz::song_name + 1];
        memset(s_song, ' ', sz::number + 1);
        memcpy(s_song + sz::number, song_name[i], sz::song_name);
        s_song[sz::number + sz::song_name] = 0;
    
        if (to_screen)
            display_lines(i, old_pos_playing, s_song, s_group, selected, redraw_type, playlist_view_cnt, lcd_color_yellow, redraw_screen);
        if (selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_displayed_song(s_group, s_song, selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

