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
    
    char playlist_name[plb_view_cnt][pl_name_sz + 1];
    char number[plb_view_cnt][3 + 1];
    char count[plb_view_cnt][3 + 1];
    char selected[plb_view_cnt];
 
    char empty[name_offset + pl_name_sz + count_offset + 3 + 1];
    memset(empty, ' ', sizeof(empty));
    empty[count_offset + 3] = 0;
    
    pll.print(playing_pl, playlist_name, number, count, selected);
    redraw_type_t redraw_type = pll.redraw_type();
    pll.reset_display();

    if (to_screen)
        scroll_text <plb_view_cnt> (redraw_type);  
    static uint32_t old_pos_playing = plb_view_cnt;
    
    uint32_t pos_playing = plb_view_cnt;
    for (uint32_t i = 0; i != plb_view_cnt; ++i)
    {
        
        char s_playlist[name_offset + pl_name_sz + count_offset + 3 + 1];
        memset(s_playlist, ' ', sizeof(s_playlist));
        memcpy(s_playlist, number[i], 3);
        memcpy(s_playlist + name_offset, playlist_name[i], pl_name_sz);
        memcpy(s_playlist + count_offset, count[i], 3);
        s_playlist[count_offset + 3] = 0;

        if (to_screen)
            display_lines(i, old_pos_playing, s_playlist, empty, selected, redraw_type, plb_view_cnt, lcd_color_green, redraw_screen);
        if (selected[i] & 2)
            pos_playing = i; 
        
        audio_ctl.audio_process(need_redraw);
        send_pl_list(s_playlist, selected[i], i);
    }
    old_pos_playing = pos_playing;
}

}

