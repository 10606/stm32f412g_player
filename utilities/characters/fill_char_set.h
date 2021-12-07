#ifndef FILL_CHAR_SET_H
#define FILL_CHAR_SET_H

#include <stdint.h>
#include <array>
#include <unordered_map>

void fill_char_set (std::array <uint32_t, 256> & char_set) noexcept;
std::unordered_map <uint32_t, uint8_t> rev_char_map (std::array <uint32_t, 256> const & char_set);
std::unordered_map <uint32_t, uint8_t> fill_rev_char_map ();

#endif

