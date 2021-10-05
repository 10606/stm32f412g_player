#include "usb_command_process.h"

#include "view.h"
#include "lcd_display.h"
#include <algorithm>
#include <type_traits>
#include <stdint.h>

uint32_t usb_process_t::usb_process (view * vv)
{
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
        &view::to_end_and_pause
    };
    
    for (; start != end;)
    {
        [[maybe_unused]] uint32_t ret = 0;
        uint8_t command = buffer[start];
        if (command < std::extent <decltype(process_view_do)> ::value)
            ret = (vv->*process_view_do[command])();
        start = (start + 1) % std::extent <decltype(buffer)> ::value;
    }
    
    return 0;
}

void usb_process_t::receive_callback (volatile uint8_t * buf, uint32_t len)
{
    uint32_t n = std::extent <decltype(buffer)> ::value;
    uint32_t tmp = (end_buf + 1) % n;
    uint32_t i = 0;
    for (i = 0; (tmp != start) && (i != len); i++)
    {
        if (need_rd + need_skip == 0)
            need_rd = calc_need_rd(buf[i]);
        
        if (need_skip != 0)
        {
            need_skip = need_skip - 1;
            continue;
        }
        
        buffer[end_buf] = buf[i];
        end_buf = tmp;
        need_rd = need_rd - 1;
        tmp = (tmp + 1) % n;
        
        if (need_rd == 0)
            end = end_buf;
    }
    
    if (i != len)
    {
        end_buf = end;
        need_skip = need_rd;
    }
    
    for (; i != len; ++i)
    {
        if (need_skip != 0)
            need_skip = need_skip - 1;
        else
            need_skip = calc_need_rd(buf[i]) - 1;
    }
}

uint32_t usb_process_t::calc_need_rd (uint8_t first_byte)
{
    return 1;
}

usb_process_t::usb_process_t ()
{
    clear();
}

void usb_process_t::clear ()
{
    start = 0;
    end = 0;
    end_buf = 0;
    need_skip = 0;
    need_rd = 0;
}

usb_process_t usb_process_v;

