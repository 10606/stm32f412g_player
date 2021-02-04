#include "usb_command_process.h"

#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"
#include <stdint.h>

uint8_t usb_process_buffer[4];
uint32_t usb_process_start = 0;
uint32_t usb_process_end = 0;

uint32_t usb_process (view * vv, uint8_t * need_redraw)
{
    for (; usb_process_start != usb_process_end;)
    {
        uint32_t ret = 0;
        uint8_t need_redraw_nv = 0;
        switch (usb_process_buffer[usb_process_start])
        {
            case command_up:
                ret = process_view_up(vv, &need_redraw_nv);
                break;
            case command_down:
                ret = process_view_down(vv, &need_redraw_nv);
                break;
                
            case command_back:
                ret = process_view_left(vv, &need_redraw_nv);
                break;
            case command_forward:
                ret = process_view_center(vv, &need_redraw_nv);
                break;
            case command_select:
                ret = process_view_right(vv, &need_redraw_nv);
                break;
                
            case commnad_play_pause:
                ret = process_view_play_pause(vv, &need_redraw_nv);
                break;
            case command_repeat:
                ret = process_toggle_repeat(vv, &need_redraw_nv);
                break;
                
            case commnad_volume_up:
                ret = process_view_inc_volume(vv, &need_redraw_nv);
                break;
            case command_volume_down:
                ret = process_view_dec_volume(vv, &need_redraw_nv);
                break;
             
            case command_seek_forward:
                ret = process_view_seek_forward(vv, &need_redraw_nv);
                break;
            case command_seek_backward:
                ret = process_view_seek_backward(vv, &need_redraw_nv);
                break;
                
            case command_next_song:
                ret = process_view_next_song(vv, &need_redraw_nv);
                break;
            case commnad_prev_song:
                ret = process_view_prev_song(vv, &need_redraw_nv);
                break;
                
            case commnad_send_info:
                need_redraw_nv = 1;
                //display_song_volume(vv.pl, viewer.audio_ctl, vv.state_song_view, 0);
                break;
                
            default:
                break;
        }
        
        *need_redraw |= need_redraw_nv;
        
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

