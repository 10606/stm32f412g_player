#ifndef CHECK_STRING_H
#define CHECK_STRING_H

#include <string_view>
#include <stddef.h>

const size_t diff_sz_limit = 100;
const size_t activation_porog = 10;

size_t diff_string (std::string_view const a, std::string_view const b);

#endif

