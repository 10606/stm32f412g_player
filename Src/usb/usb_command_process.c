#include "usb_command_process.h"

#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"
#include <stdint.h>

extern volatile uint8_t need_redraw;
extern view viewer;

uint8_t usb_process_buffer[4];
uint32_t usb_process_start = 0;
uint32_t usb_process_end = 0;

uint32_t usb_process ()
{
    for (; usb_process_start != usb_process_end;)
    {
        uint32_t ret = 0;
        uint8_t need_redraw_nv = 0;
        switch (usb_process_buffer[usb_process_start])
        {
            case command_up:
                ret = process_view_up(&viewer, &need_redraw_nv);
                break;
            case command_down:
                ret = process_view_down(&viewer, &need_redraw_nv);
                break;
                
            case command_back:
                ret = process_view_left(&viewer, &need_redraw_nv);
                break;
            case command_forward:
                ret = process_view_center(&viewer, &need_redraw_nv);
                break;
            case command_select:
                ret = process_view_right(&viewer, &need_redraw_nv);
                break;
                
            case commnad_play_pause:
                ret = process_view_play_pause(&viewer, &need_redraw_nv);
                break;
            case command_repeat:
                viewer.buffer_ctl->repeat_mode ^= 1;
                display_song_volume(&viewer.pl, viewer.buffer_ctl, &viewer.state_song_view, 1);
                break;
                
            case commnad_volume_up:
                ret = process_view_inc_volume(&viewer, &need_redraw_nv);
                break;
            case command_volume_down:
                ret = process_view_dec_volume(&viewer, &need_redraw_nv);
                break;
             
            case command_seek_forward:
                ret = process_view_seek_forward(&viewer, &need_redraw_nv);
                break;
            case command_seek_backward:
                ret = process_view_seek_backward(&viewer, &need_redraw_nv);
                break;
                
            case command_next_song:
                ret = process_view_next_song(&viewer, &need_redraw_nv);
                break;
            case commnad_prev_song:
                ret = process_view_prev_song(&viewer, &need_redraw_nv);
                break;
                
            case commnad_send_info:
                need_redraw_nv = 1;
                //display_song_volume(&viewer.pl, viewer.buffer_ctl, &viewer.state_song_view, 0);
                break;
                
            default:
                break;
        }
        
        need_redraw |= need_redraw_nv;
        
        if (!ret)
            usb_process_start = (usb_process_start + 1) % sizeof(usb_process_buffer);
    }
    return 0;
}

uint32_t receive_callback (volatile uint8_t * buf, uint32_t len)
{
    uint32_t tmp = (usb_process_end + 1) % sizeof(usb_process_buffer);
    if (tmp == usb_process_start)
        return 1;
    usb_process_buffer[usb_process_end] = buf[0];
    usb_process_end = tmp;
    return 1;
}

