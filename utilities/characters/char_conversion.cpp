#include "char_conversion.h"

#include <vector>
#include <string>
#include <stdexcept>

std::vector <uint32_t> utf8_to_ucs4 (std::string_view const & utf8)
{
    std::vector <uint32_t> answer;
    answer.reserve(utf8.size());
    for (size_t i = 0; i != utf8.size();)
    {
        uint8_t c = utf8[i++];
        uint32_t ans;
        uint8_t rem;
        if (c < 0x80) [[likely]] 
        {
            ans = c;
            rem = 0;
        }
        else if (c < 0xc0) [[unlikely]] 
        {
            continue;
            // ignore err
        }
        else if (c < 0xe0) [[likely]] 
        {
            ans = c & 0x1f;
            rem = 1;
        }
        else if (c < 0xf0) [[likely]] 
        {
            ans = c & 0x0f;
            rem = 2;
        }
        else if (c < 0xf8) [[likely]] 
        {
            ans = c & 0x07;
            rem = 3;
        }
        else
        {
            continue;
            // ignore err
        }
        
        for (; rem && i != utf8.size(); ++i)
        {
            c = utf8[i];
            ans <<= 6;
            ans += c & 0x3f;
            rem--;
        }
        
        if (rem ||
            (ans >= 0xd800 && ans < 0xe000) ||
            (ans >= 0x110000)) [[unlikely]] 
        {
            continue;
            // ignore err
        }
        answer.push_back(ans);
    }
    return answer;
}

std::basic_string <uint16_t> utf8_to_utf16 (std::string_view const & utf8)
{
    std::vector <uint32_t> unicode = utf8_to_ucs4(utf8);
    std::basic_string <uint16_t> utf16;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni < 0x10000)
        {
            utf16 += (uint16_t)uni;
        }
        else
        {
            uni -= 0x10000;
            utf16 += (uint16_t)((uni >> 10) + 0xd800);
            utf16 += (uint16_t)((uni & 0x3ff) + 0xdc00);
        }
    }
    return utf16;
}

std::basic_string <uint16_t> utf8_to_ucs2 (std::string_view const & utf8)
{
    std::vector <uint32_t> unicode = utf8_to_ucs4(utf8);
    std::basic_string <uint16_t> ucs2;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni <= 0xffff)
            ucs2 += (uint16_t)uni;
        else
        {
            // skip
        }
    }
    return ucs2;
}

std::string ucs4_to_utf8 (std::vector <uint32_t> const & ucs4)
{
    std::string answer;
    for (size_t i = 0; i != ucs4.size();)
    {
        char buf[68];
        size_t p = 0;
        for (; p < 64 && i != ucs4.size(); ++i)
        {
            uint32_t c = ucs4[i];
            if (c < 0x80) [[likely]] 
            {
                buf[p++] = (c & 0x7f);
            }
            else if (c < 0x800) [[likely]] 
            {
                buf[p++] = ((c >> 6) | 0xc0);
                buf[p++] = ((c >> 0) & 0x3f) | 0x80;
            }
            else if (c < 0x00010000) [[likely]] 
            {
                if (c >= 0xd800 && c < 0xe000) [[unlikely]] 
                    continue; // err
                
                buf[p++] = ((c >> 12) | 0xe0);
                buf[p++] = ((c >>  6) & 0x3f) | 0x80;
                buf[p++] = ((c >>  0) & 0x3f) | 0x80;
            }
            else if (c < 0x00110000) [[likely]] 
            {
                buf[p++] = ((c >> 18) | 0xf0);
                buf[p++] = ((c >> 12) & 0x3f) | 0x80;
                buf[p++] = ((c >>  6) & 0x3f) | 0x80;
                buf[p++] = ((c >>  0) & 0x3f) | 0x80;
            }
            else
            {
                // ignore err
            }
        }
        answer += std::string(buf, p);
    }
    return answer;
}

