#include "utf8_automat.h"

utf8_automat::state_t utf8_automat::next (char _c) noexcept
{
    unsigned char c = static_cast <unsigned char> (_c);
    if (symbols) [[unlikely]] 
    {
        if ((c > 0xbf) || (c < 0x80)) [[unlikely]] 
        {
            symbols = 0;
            ans = 0;
            return err;
        }
        ans <<= 6;
        ans += c & 0x3f;
        symbols--;
        if (!symbols)
        {
            if ((ans >= 0xd800 && ans < 0xe000) ||
                (ans >= 0x110000)) [[unlikely]] 
            {
                ans = 0;
                return err;
            }
            return ready;
        }
    }
    else
    {
        if (c < 0x80) [[likely]] 
        {
            ans = c;
            return ready;
        }
        else if (c < 0xc0) [[unlikely]] 
        {
            ans = 0;
            return err;
        }
        else if (c < 0xe0) [[likely]] 
        {
            ans = c & 0x1f;
            symbols = 1;
        }
        else if (c < 0xf0) [[likely]] 
        {
            ans = c & 0x0f;
            symbols = 2;
        }
        else if (c < 0xf8) [[likely]] 
        {
            ans = c & 0x07;
            symbols = 3;
        }
        else
        {
            ans = 0;
            return err;
        }
    }
    return none;
}

