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
    
    char s_song[playlist_view_cnt][sz::number + sz::song_name + 1];
    char s_group[playlist_view_cnt][sz::number + sz::group_name + 1];
    char selected[playlist_view_cnt];
    
    plv.print(pl, s_song, s_group, selected);
    redraw_type_t redraw_type = plv.redraw_type();
    plv.reset_display();

    if (to_screen)
        scroll_text <playlist_view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = playlist_view_cnt;
    
    uint32_t pos_playing = playlist_view_cnt;
    for (uint32_t i = 0; i != playlist_view_cnt; ++i)
    {
        if (to_screen)
            display_lines(i, old_pos_playing, s_song[i], s_group[i], selected, redraw_type, playlist_view_cnt, lcd_color_yellow, redraw_screen);
        if (selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_displayed_song(s_group[i], s_song[i], selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

