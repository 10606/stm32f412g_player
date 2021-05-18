#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

char check_near (uint32_t pos_a, uint32_t pos_b, uint32_t max_pos_a, uint32_t view_pos_a, uint32_t border_pos_a); // b in window a
void sprint (char * dst, size_t sz, const char * format, uint32_t value); // dst has size [sz + 1]
void sprint_mod_1000 (char * dst, size_t sz, uint32_t value); // dst has size [sz + 1]

#endif

