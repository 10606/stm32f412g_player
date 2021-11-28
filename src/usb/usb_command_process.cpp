#include "usb_command_process.h"

#include "view.h"
#include "lcd_display.h"
#include "usb_commands.h"
#include <algorithm>
#include <type_traits>
#include <stdint.h>

template <typename T, ret_code (view::* func) (T)>
ret_code just_do_this (view * vv, uint8_t * data)
{
    std::decay_t <T> value;
    memmove(&value, data, sizeof(value));
    return (vv->*func)(value);
};

ret_code usb_process_t::usb_process (view * vv)
{
    static ret_code (view::* process_view_do[18]) () =
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
        &view::to_end_and_pause,
        &view::find_next,
        &view::set_next_song
    };
    
    static ret_code (* process_view_do_arg[2]) (view *, uint8_t * data)
    {
        &just_do_this <find_pattern const &, &view::find>, // cmd_find
        &just_do_this <position_t, &view::jmp> // cmd_jmp
    };
    
    uint32_t same_commands = 0;
    uint8_t old_command = 0;
    for (; start != end;)
    {
        static const uint32_t mod = std::extent_v <decltype(buffer)>; 
        
        [[maybe_unused]] ret_code ret = 0;
        uint8_t command = buffer[start];
        
        if (command == old_command)
        {
            if (same_commands >= 4)
            {
                start = (start + calc_need_rd(command)) % mod;
                continue;
            }
            else
                same_commands++;
        }
        else
        {
            same_commands = 0;
            old_command = command;
        }
        
        if (command < std::extent_v <decltype(process_view_do)>) [[likely]]
        {
            ret = (vv->*process_view_do[command])();
        }
        else if (command >= 128 &&
                 static_cast <size_t> (command - 128) < std::extent_v <decltype(process_view_do_arg)>) [[likely]]
        {
            uint8_t cmd_data[calc_need_rd(command) - 1];
            for (uint32_t i = 0; i != calc_need_rd(command) - 1; ++i)
                cmd_data[i] = buffer[(start + i + 1) % mod];
            
            ret = process_view_do_arg[command - 128](vv, cmd_data);
        }
        start = (start + calc_need_rd(command)) % mod;
    }
    
    return 0;
}

void usb_process_t::receive_callback (volatile uint8_t * buf, uint32_t len)
{
    has_interrupted = 1;
    uint32_t n = std::extent <decltype(buffer)> ::value;
    uint32_t tmp = (end_buf + 1) % n;
    uint32_t i = 0;
    for (i = 0; (tmp != start) && (i != len); i++)
    {
        if (need_rd + need_skip == 0) [[likely]]
            need_rd = calc_need_rd(buf[i]);
        
        if (need_skip != 0) [[unlikely]]
        {
            need_skip = need_skip - 1;
            continue;
        }
        
        buffer[end_buf] = buf[i];
        end_buf = tmp;
        need_rd = need_rd - 1;
        tmp = (tmp + 1) % n;
        
        if (need_rd == 0) [[likely]]
            end = end_buf;
    }
    
    if (i != len) [[unlikely]]
    {
        end_buf = end;
        need_skip = need_rd;
    }
    
    for (; i != len; ++i)
    {
        if (need_skip != 0) [[unlikely]]
            need_skip = need_skip - 1;
        else
            need_skip = calc_need_rd(buf[i]) - 1;
    }
}

usb_process_t::usb_process_t ()
{
    clear();
}

void usb_process_t::clear ()
{
    do
    {
        has_interrupted = 0;
        start = 0;
        end = 0;
        end_buf = 0;
        need_skip = 0;
        need_rd = 0;
    }
    while (has_interrupted);
}

usb_process_t usb_process_v;

