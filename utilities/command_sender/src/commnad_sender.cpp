#include "usb_commands.h"

#include "command_line/send_by_name.h"
#include "interactive/interactive.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

int main (int argc, char ** argv)
{
    int fd = -1;
    
    try
    {
        //const std::string stm32_path("/dev/serial/by-id/usb-STMicroelectronics_STM32_Virtual_ComPort_313FB9553136-if00");
        //std::ofstream stm32(stm32_path);

        char const * sock_name = "/home/wa51/code/code_microcontrollers/player/utilities/multiplex_server/qwe.socket";
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == -1) 
            throw std::runtime_error("can't create socket");
        
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, sock_name, sizeof(addr.sun_path) - 1);

        int ret = connect(fd, reinterpret_cast <sockaddr *> (&addr), sizeof(addr));
        if (ret == -1) 
            throw std::runtime_error("can't connect");
        
        if (argc >= 2) //command mode
        {
            for (int i = 1; i != argc; ++i)
            {
                if (!send(fd, argv[i]))
                {
                    std::cerr << argv[i] << "\n";
                }
            }
        }
        else //interactive mode
        {
            std::cout << "interactive mode\n";
            interactive(fd);
        }
        
        close(fd);
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
        if (fd != -1)
            close(fd);
    }
}
