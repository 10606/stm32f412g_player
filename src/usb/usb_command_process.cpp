#include "usb_command_process.h"

#include "view.h"
#include "display.h"
#include <type_traits>
#include <stdint.h>

struct usb_process_t
{
    uint8_t buffer[4];
    uint32_t start = 0;
    uint32_t end = 0;
};

usb_process_t usb_process_v;

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
    
    for (; usb_process_v.start != usb_process_v.end;)
    {
        uint32_t ret = 0;
        uint8_t need_redraw_nv = 0;
        uint8_t command = usb_process_v.buffer[usb_process_v.start];
        if (command < 16)
            ret = (vv->*process_view_do[command])(&need_redraw_nv);
        *need_redraw |= need_redraw_nv;
        if (!ret)
            usb_process_v.start = (usb_process_v.start + 1) % std::extent <decltype(usb_process_v.buffer)> ::value;
    }
    return 0;
}

void receive_callback (volatile uint8_t * buf, uint32_t len)
{
    uint32_t n = std::extent <decltype(usb_process_v.buffer)> ::value;
    uint32_t tmp = (usb_process_v.end + 1) % n;
    for (uint32_t i = 0; tmp != usb_process_v.start && i != len; i++, tmp = (tmp + 1) % n)
    {
        usb_process_v.buffer[usb_process_v.end] = buf[i];
        usb_process_v.end = tmp;
    }
}

