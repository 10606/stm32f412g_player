#include "send_by_name.h"
#include "check_string.h"
#include "usb_commands.h"
#include "string_command.h"
#include <fstream>
#include <string>
#include <limits>
#include <type_traits>
#include <iostream>

bool send (std::ofstream & f, std::string const & command)
{
    size_t best_v = std::numeric_limits <size_t> :: max();
    unsigned char best_i = 0;
    
    for (unsigned char i = 0; i != std::extent <decltype(str_commands)>::value; ++i)
    {
        size_t tmp = diff_string(command, str_commands[i]);
        if (best_v > tmp)
        {
            best_v = tmp;
            best_i = i;
        }
    }
    
    if (best_v < activation_porog)
    {
        if (best_v != 0)
        {
            std::cout << command << " -> " << str_commands[best_i];
        }
        f.put(best_i);
        return 1;
    }
    return 0;
}
