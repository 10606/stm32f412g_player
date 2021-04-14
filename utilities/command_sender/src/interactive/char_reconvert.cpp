#include "char_reconvert.h"

#include <string_view>
#include <string>
#include <array>
#include "fill_char_set.h"
#include "char_conversion.h"

struct char_set_t
{
    char_set_t ()
    {
        fill_char_set(value);
    }
    
    std::array <uint32_t, 256> value;
};

std::string char_reconvert (std::string_view const & value)
{
    static char_set_t char_set;
    
    std::vector <uint32_t> ucs4;
    ucs4.reserve(value.size());
    for (char c : value)
    {
        ucs4.push_back(char_set.value[static_cast <uint8_t> (c)]);
    }
    
    return ucs4_to_utf8(ucs4);
}

