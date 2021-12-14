#include "calc_utf8_len.h"

size_t calc_utf8_len (std::string const & value)
{
    size_t ans = 0;
    // errors increase answer...
    uint8_t need = 0;
    
    for (uint8_t c : value)
    {
        if (need)
        {
            need--;
            if (c >> 6 == 0b10)
                continue;
            need = 0;
        }
        if (c < 0x80)
        {
            // non printable
            ans += (c >= 0x20);
            continue;
        }
        if (c >> 5 <= 0b110)
        {
            switch (c >> 5)
            {
            case 0b110:
                need = 1;
            case 0b100:
            case 0b101:
                ans++;
            }
        }
        else
        {
            switch (c >> 3)
            {
            case 0b11110:
                need += 1;
            case 0b11100:
            case 0b11101:
                need += 2;
            case 0b11111:
                ans++;
            }
        }
    }

    return ans;
}

