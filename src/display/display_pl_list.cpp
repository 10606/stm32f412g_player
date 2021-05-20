#include "display_playlists.h"

#include "display_common.h"
#include "lcd_display.h"
#include "audio.h"
#include "pl_list.h"
#include "usb_send.h"
#include <stdint.h>

namespace display
{

void cur_pl_list 
(
    pl_list & pll, 
    uint32_t playing_pl, 
    state_t cur_state, 
    state_t old_state, 
    bool & need_redraw
)
{
    bool to_screen = cur_state == state_t::pl_list;
    if (to_screen)
        fill_borders <pl_list::view_cnt> ();
    
    pl_list::print_info print = pll.print(playing_pl);
    redraw_type_t redraw_type = pll.redraw_type();
    pll.reset_display();
    
    char empty[sz::count_offset + sz::count + 1];
    memset(empty, ' ', sizeof(empty));
    empty[sizeof(empty) - 1] = 0;

    if (to_screen)
        scroll_text <pl_list::view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = pl_list::view_cnt;
    
    uint32_t pos_playing = pl_list::view_cnt;
    for (uint32_t i = 0; i != pl_list::view_cnt; ++i)
    {
        if (to_screen && need_draw_line(i, old_pos_playing, print.selected, redraw_type, pl_list::view_cnt, old_state != state_t::pl_list)) 
            display_lines(i, print.playlist_name[i], empty, print.selected, lcd_color_green);
        if (print.selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_pl_list(print.playlist_name[i], print.selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

