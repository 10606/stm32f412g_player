#include "utf8_automat.h"

utf8_automat::state_t utf8_automat::next (char _c)
{
    unsigned char c = static_cast <unsigned char> (_c);
    if (symbols)
    {
        if ((c > 0xbf) || (c < 0x80))
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
            if ((ans >= 0xd800 && ans <= 0xdfff) ||
                (ans > 0x10ffff))
            {
                ans = 0;
                return err;
            }
            return ready;
        }
    }
    else
    {
        if (c < 0x80)
        {
            ans = c;
            return ready;
        }
        else if (c < 0xc0)
        {
            ans = 0;
            return err;
        }
        else if (c < 0xe0)
        {
            ans = c & 0x1f;
            symbols = 1;
        }
        else if (c < 0xf0)
        {
            ans = c & 0x0f;
            symbols = 2;
        }
        else if (c < 0xf8)
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

