#include "usb_command_process.h"

#include "view.h"
#include <type_traits>
#include <stdint.h>

uint32_t usb_process_t::usb_process (view * vv, bool & need_redraw)
{
    static uint32_t (view::* process_view_do[16]) (bool &) =
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
    
    for (; start != end;)
    {
        uint32_t ret = 0;
        uint8_t command = buffer[start];
        if (command < 16)
            ret = (vv->*process_view_do[command])(need_redraw);
        if (!ret)
            start = (start + 1) % std::extent <decltype(buffer)> ::value;
    }
    return 0;
}

void usb_process_t::receive_callback (volatile uint8_t * buf, uint32_t len)
{
    uint32_t n = std::extent <decltype(buffer)> ::value;
    uint32_t tmp = (end + 1) % n;
    for (uint32_t i = 0; tmp != start && i != len; i++, tmp = (tmp + 1) % n)
    {
        buffer[end] = buf[i];
        end = tmp;
    }
}

void usb_process_t::clear ()
{
    start = 0;
    end = 0;
}

usb_process_t usb_process_v;

