#include "usb_command_process.h"

#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"
#include <stdint.h>

uint8_t usb_process_buffer[4];
uint32_t usb_process_start = 0;
uint32_t usb_process_end = 0;

static uint32_t process_view_do_nothing (view * vv, uint8_t * need_redraw)
{
    return 0;
}

static uint32_t process_view_send_info (view * vv, uint8_t * need_redraw)
{
    *need_redraw = 1;
    return 0;
}

uint32_t usb_process (view * vv, uint8_t * need_redraw)
{
    static uint32_t (* process_view_do[16]) (view *, uint8_t *) =
    {
        process_view_do_nothing,
        process_view_up,
        process_view_down,
        process_view_left,
        process_view_center,
        process_view_right,
        process_view_play_pause,
        process_view_inc_volume,
        process_view_dec_volume,
        process_toggle_repeat,
        process_view_seek_forward,
        process_view_seek_backward,
        process_view_next_song,
        process_view_prev_song,
        process_view_send_info,
        process_view_do_nothing
    };
    
    for (; usb_process_start != usb_process_end;)
    {
        uint32_t ret = 0;
        uint8_t need_redraw_nv = 0;
        uint8_t command = usb_process_buffer[usb_process_start];
        if (command < 16)
            ret = process_view_do[command](vv, &need_redraw_nv);
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

