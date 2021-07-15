#include <unistd.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <string_view>
#include <vector>
#include <stdexcept>
#include "epoll_wrapper.h"
#include "unix_server_sock.h"
#include "com_wrapper.h"
#include "client_wrapper.h"

void set_sig_handler (int sig_num, void (* handler) (int))
{
    struct sigaction act = {handler, 0, 0, 0, 0};
    int ret = sigaction(sig_num, &act, NULL);
    if (ret)
        std::runtime_error("can't set signal handler");
}

int main (int argc, char ** argv)
{
    char const * sock_name = "/home/wa51/code/code_microcontrollers/player/utilities/multiplex_server/qwe.socket";
    char const * stm32_name = "/dev/serial/by-id/usb-STMicroelectronics_player_stm32f412g_313FB9553136-if00";
 
    bool socket_activation = (argc > 1) && (strcmp(argv[1], "--socket_activation") == 0);
 
    set_sig_handler(SIGPIPE, SIG_IGN);
    
    try
    {
        epoll_wraper epoll_wrap;
        unix_server_sock_t conn_sock(sock_name, socket_activation, epoll_wrap.fd());
            
        try
        {
            clients_wrapper_t clients(epoll_wrap.fd());
            com_wrapper_t stm32(stm32_name, epoll_wrap.fd());

            bool exit = 0;
            while (!exit)
            {
                std::vector <std::pair <int, uint32_t> > events = epoll_wrap.wait();
                for (std::pair <int, uint32_t> const & event : events)
                {
                    if (event.first == conn_sock.file_descriptor())
                    {
                        if (event.second & EPOLLIN)
                            clients.reg(conn_sock.accept());
                    }
                    else if (event.first == stm32.file_descriptor())
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

                for (std::pair <int, uint32_t> const & event : events)
                {
                    static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                    if (event.second & err_mask)
                    {
                        epoll_wrap.unreg(event.first);
                        if (event.first == conn_sock.file_descriptor())
                            throw std::runtime_error("server socket error");
                        else if (event.first == stm32.file_descriptor())
                            exit = 1;
                        else
                            clients.unreg(event.first);
                    }
                }
            }
        }
        catch (std::exception const & e)
        {
            std::cerr << e.what() << '\n';
            // accept all clients and return
            //  otherwise 
            //      client wait on connect
            //      systemd will restart me
            //      (((
            try
            {
                bool flag = 1;
                
                while (flag)
                {
                    flag = 0;
                    std::vector <std::pair <int, uint32_t> > events = epoll_wrap.wait(1000);
                    for (std::pair <int, uint32_t> const & event : events)
                    {
                        if ((event.first == conn_sock.file_descriptor()) &&
                            (event.second & EPOLLIN))
                        {
                            flag = 1;
                            int fd = conn_sock.accept();
                            if (fd != -1)
                                close(fd);
                        }
                    }
                }
            }
            catch (std::exception const & e)
            {
                std::cerr << e.what() << '\n';
                return 1;
            }
            return 0;
        }
        catch (...)
        {
            std::cerr << "unknown error";
            return 2;
        }
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
        return 3;
    }
    catch (...)
    {
        std::cerr << "unknown error";
        return 4;
    }
}


