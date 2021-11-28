#ifndef HEADERS_H
#define HEADERS_H

#include <cstddef>
#include <string>
#include <iostream>
#include <deque>
#include <array>

#include "usb_commands.h"

void extract (std::deque <char> & data, state_t & state);
void display_search (std::array <std::string, 2> const & value);
void display_number (position_t value);

#endif

