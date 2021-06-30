#include "usb_command_process.h"

#include "lcd_display.h" // TODO remove

#include "view.h"
#include <type_traits>
#include <stdint.h>

uint32_t usb_process_t::usb_process (view * vv)
{
    uint32_t n = std::extent <decltype(buffer)> ::value;
    static uint32_t (view::* process_view_do[16]) () =
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
        [[maybe_unused]] uint32_t ret = 0;
        if (((start + 1) % n == end) && // "^E^@^@^@" control sequences
            (buffer[start] == 0x5e))
        {
            break;
        }
        
        uint8_t command = buffer[start];
        if (command < 16)
            ret = (vv->*process_view_do[command])();
        start = (start + 1) % std::extent <decltype(buffer)> ::value;
    }
    return 0;
}

void usb_process_t::receive_callback (uint8_t * buf, uint32_t len)
{
    uint32_t n = std::extent <decltype(buffer)> ::value;
    uint32_t tmp = (end + 1) % n;
    for (uint32_t i = 0; (tmp != start) && (i != len); i++)
    {
        buffer[end] = buf[i];
        end = tmp;
        tmp = (tmp + 1) % n;
        
        if (((start + 1) % n != end) && // "^E^@^@^@" control sequences
            (buffer[start] == 0x5e))
        {
            start = (start + 2) % n;
        }
    }
}

usb_process_t::usb_process_t ()
{
    clear();
}

void usb_process_t::clear ()
{
    start = 0;
    end = 0;
}

usb_process_t usb_process_v;

