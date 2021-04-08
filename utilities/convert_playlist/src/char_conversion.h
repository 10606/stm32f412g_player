#ifndef CHAR_CONVERSION_H
#define CHAR_CONVERSION_H

#include <vector>
#include <stdint.h>
#include <string>

std::vector <uint32_t> utf8_to_ucs4 (std::string const & utf8);
std::basic_string <uint16_t> utf8_to_utf16 (std::string const & utf8);
std::basic_string <uint16_t> utf8_to_ucs2 (std::string const & utf8);

#endif

