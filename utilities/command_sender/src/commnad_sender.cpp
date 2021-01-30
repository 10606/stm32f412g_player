#include "usb_commands.h"

#include "command_line/send_by_name.h"
#include "interactive/interactive.h"

#include <iostream>
#include <fstream>

int main (int argc, char ** argv)
{
    try
    {
        const std::string stm32_path("/dev/serial/by-id/usb-STMicroelectronics_STM32_Virtual_ComPort_313FB9553136-if00");
        std::ofstream stm32(stm32_path);
        if (argc >= 2) //command mode
        {
            for (int i = 1; i != argc; ++i)
            {
                if (!send(stm32, argv[i]))
                {
                    std::cerr << argv[i] << " ";
                }
            }
        }
        else //interactive mode
        {
            std::cout << "interactive mode\n";
            interactive(stm32_path);
        }
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
    }
}
