#include "char_conversion.h"

#include <vector>
#include <string>
#include <stdexcept>
#include "utf8_automat.h"

std::vector <uint32_t> utf8_to_ucs4 (std::string const & utf8)
{
    std::vector <uint32_t> unicode;
    utf8_automat aut;
    for (size_t i = 0; i != utf8.size(); ++i)
    {
        utf8_automat::state_t st = aut.next(utf8[i]);
        if (st == utf8_automat::err)
            throw std::runtime_error("not a UTF-8 string");
        if (st == utf8_automat::none)
        {
            if (i + 1 == utf8.size())
                throw std::runtime_error("not a UTF-8 string (unexpected end)");
            continue;
        }
        uint32_t uni = aut.get_ans();
        unicode.push_back(uni);
    }
    return unicode;
}

std::basic_string <uint16_t> utf8_to_utf16 (std::string const & utf8)
{
    std::vector <uint32_t> unicode = utf8_to_ucs4(utf8);
    std::basic_string <uint16_t> utf16;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni <= 0xFFFF)
        {
            utf16 += (uint16_t)uni;
        }
        else
        {
            uni -= 0x10000;
            utf16 += (uint16_t)((uni >> 10) + 0xD800);
            utf16 += (uint16_t)((uni & 0x3FF) + 0xDC00);
        }
    }
    return utf16;
}

std::basic_string <uint16_t> utf8_to_ucs2 (std::string const & utf8)
{
    std::vector <uint32_t> unicode = utf8_to_ucs4(utf8);
    std::basic_string <uint16_t> ucs2;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni <= 0xFFFF)
            ucs2 += (uint16_t)uni;
        else
        {
            // skip
        }
    }
    return ucs2;
}


