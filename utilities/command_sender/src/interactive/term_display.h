#ifndef HEADERS_H
#define HEADERS_H

#include <cstddef>
#include <string>
#include <iostream>
#include <deque>

#include "usb_commands.h"

void extract (std::deque <char> & data, state_t & state);

#endif

