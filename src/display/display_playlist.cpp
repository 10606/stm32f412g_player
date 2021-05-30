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
    state_t cur_state,
    state_t old_state,
    bool & need_redraw
)
{
    bool to_screen = cur_state == state_t::playlist;
    if (to_screen)
        fill_borders <playlist_view::view_cnt> ();
    HAL_Delay(1);
    audio_ctl.audio_process(need_redraw);
    
    playlist_view::print_info print = plv.print(pl);
    redraw_type_t redraw_type = plv.redraw_type();
    plv.reset_display();

    if (to_screen)
        scroll_text <playlist_view::view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = playlist_view::view_cnt;
    
    uint32_t pos_playing = playlist_view::view_cnt;
    for (uint32_t i = 0; i != playlist_view::view_cnt; ++i)
    {
        if (to_screen && need_draw_line(i, old_pos_playing, print.selected, redraw_type, playlist_view::view_cnt, old_state != state_t::playlist)) 
            display_lines(i, print.group_name[i], print.song_name[i], print.selected, lcd_color_yellow);
        if (print.selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_displayed_song(print.group_name[i], print.song_name[i], print.selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

