#include "usb_command_process.h"

#include "view.h"
#include "display.h"
#include <stdint.h>

uint8_t usb_process_buffer[4];
uint32_t usb_process_start = 0;
uint32_t usb_process_end = 0;

uint32_t usb_process (view * vv, uint8_t * need_redraw)
{
    static uint32_t (view::* process_view_do[16]) (uint8_t *) =
    {
        &view::do_nothing,
        &view::process_up,
        &view::process_down,
        &view::process_left,
        &view::process_center,
        &view::process_right,
        &view::play_pause,
        &view::inc_volume,
        &view::dec_volume,
        &view::toggle_repeat,
        &view::seek_forward,
        &view::seek_backward,
        &view::next_song,
        &view::prev_song,
        &view::send_info,
        &view::do_nothing
    };
    
    for (; usb_process_start != usb_process_end;)
    {
        uint32_t ret = 0;
        uint8_t need_redraw_nv = 0;
        uint8_t command = usb_process_buffer[usb_process_start];
        if (command < 16)
            ret = (vv->*process_view_do[command])(&need_redraw_nv);
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

