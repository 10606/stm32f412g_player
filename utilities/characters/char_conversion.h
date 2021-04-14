#ifndef CHAR_CONVERSION_H
#define CHAR_CONVERSION_H

#include <vector>
#include <stdint.h>
#include <string>
#include <string_view>

std::vector <uint32_t> utf8_to_ucs4 (std::string_view const & utf8);
std::basic_string <uint16_t> utf8_to_utf16 (std::string_view const & utf8);
std::basic_string <uint16_t> utf8_to_ucs2 (std::string_view const & utf8);
std::string ucs4_to_utf8 (std::vector <uint32_t> const & ucs4);

#endif

