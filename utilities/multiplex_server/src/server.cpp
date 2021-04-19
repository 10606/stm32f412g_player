#include <unistd.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <iostream>
#include <string_view>
#include <vector>
#include <stdexcept>
#include "epoll_wrapper.h"
#include "unix_server_sock.h"
#include "com_wrapper.h"
#include "client_wrapper.h"

int main (int argc, char ** argv)
{
    char const * sock_name = "/home/wa51/code/code_microcontrollers/player/utilities/multiplex_server/qwe.socket";
    char const * stm32_name = "/dev/serial/by-id/usb-STMicroelectronics_STM32_Virtual_ComPort_313FB9553136-if00";
 
    bool socket_activation = (argc > 1) && (strcmp(argv[1], "--socket_activation") == 0);
    
    try
    {
        epoll_wraper epoll_wrap;
        unix_server_sock_t conn_sock(sock_name, socket_activation, epoll_wrap.epoll_fd);
        clients_wrapper_t clients(epoll_wrap.epoll_fd);
        com_wrapper_t stm32(stm32_name, epoll_wrap.epoll_fd);

        while (1)
        {
            std::vector <std::pair <int, uint32_t> > events = epoll_wrap.wait();
            for (std::pair <int, uint32_t> const & event : events)
            {
                static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                if (event.second & err_mask)
                {
                    epoll_wrap.unreg(event.first);
                    if (event.first == conn_sock.fd)
                        throw std::runtime_error("server socket error");
                    else if (event.first == stm32.fd)
                        throw std::runtime_error("com closed");
                    else
                        clients.unreg(event.first);
                    continue;
                }
                
                if (event.first == conn_sock.fd)
                {
                    if (event.second & EPOLLIN)
                        clients.reg(conn_sock.accept());
                }
                else if (event.first == stm32.fd)
                {
                    if (event.second & EPOLLIN)
                        clients.append(stm32.read());
                    else if (event.second & EPOLLOUT)
                        stm32.write();
                }
                else // client's socket
                {
                    if (event.second & EPOLLIN)
                        stm32.append(clients.read(event.first));
                    else if (event.second & EPOLLOUT)
                        clients.write(event.first);
                }
            }
        }
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "unknown error";
        return 2;
    }
}


