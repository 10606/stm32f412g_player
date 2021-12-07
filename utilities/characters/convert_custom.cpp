#include "convert_custom.h"

#include <string>
#include <map>
#include "char_conversion.h"
#include "fill_char_set.h"

std::basic_string <uint8_t> utf8_to_custom (std::string const & value)
{
    static std::unordered_map <uint32_t, uint8_t> rev_map = fill_rev_char_map();
    std::basic_string <uint8_t> ans;
    std::vector <uint32_t> ucs4 = utf8_to_ucs4(value);
    ans.reserve(ucs4.size());
    std::transform(ucs4.begin(), ucs4.end(), std::inserter(ans, ans.end()),
        [] (uint32_t v) -> uint8_t 
        {
            std::unordered_map <uint32_t, uint8_t> :: const_iterator it = rev_map.find(v);
            if (it == rev_map.end())
                return 0x20; // ' '
            else
                return it->second;
        });
    return ans;
}

