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
    bool to_screen, 
    bool redraw_screen,  
    bool & need_redraw
)
{
    if (to_screen)
        fill_borders <plb_view_cnt> ();
    
    char s_playlist[plb_view_cnt][sz::count_offset + sz::count + 1];
    char selected[plb_view_cnt];
 
    char empty[sz::count_offset + sz::count + 1];
    memset(empty, ' ', sizeof(empty));
    empty[sizeof(empty) - 1] = 0;
    
    pll.print(playing_pl, s_playlist, selected);
    redraw_type_t redraw_type = pll.redraw_type();
    pll.reset_display();

    if (to_screen)
        scroll_text <plb_view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = plb_view_cnt;
    
    uint32_t pos_playing = plb_view_cnt;
    for (uint32_t i = 0; i != plb_view_cnt; ++i)
    {
        if (to_screen)
            display_lines(i, old_pos_playing, s_playlist[i], empty, selected, redraw_type, plb_view_cnt, lcd_color_green, redraw_screen);
        if (selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_pl_list(s_playlist[i], selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

