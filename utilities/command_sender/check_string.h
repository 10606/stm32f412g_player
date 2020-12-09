#ifndef CHECK_STRING_H
#define CHECK_STRING_H

#include <string>

const size_t diff_sz_limit = 100;
const size_t activation_porog = 5;

size_t diff_string (std::string const & a, std::string const & b);

#endif

