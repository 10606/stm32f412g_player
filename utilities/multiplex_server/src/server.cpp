#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <string_view>
#include <vector>
#include <stdexcept>

#include "check_password.h"
#include "tcp_server_sock.h"
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
        tcp_server_sock_t tcp_server_sock(INADDR_ANY, 750, epoll_wrap.fd());
            
        try
        {
            clients_wrapper_t clients(epoll_wrap.fd());
            com_wrapper_t stm32(stm32_name, epoll_wrap.fd());
            authentificator_t auth(epoll_wrap.fd());

            bool exit = 0;
            while (!exit)
            {
                clients.remove_too_big();
                std::vector <std::pair <int, uint32_t> > events = epoll_wrap.wait();
                for (std::pair <int, uint32_t> const & event : events)
                {
                    if (event.first == conn_sock.file_descriptor())
                    {
                        if (event.second & EPOLLIN)
                            clients.reg(conn_sock.accept());
                    }
                    else if (event.first == tcp_server_sock.file_descriptor())
                    {
                        try
                        {
                            if (event.second & EPOLLIN)
                                auth.add(tcp_server_sock.accept());
                        }
                        catch (...)
                        {}
                    }
                    else if (event.first == stm32.file_descriptor())
                    {
                        if (event.second & EPOLLIN)
                            clients.append(stm32.read());
                        if (event.second & EPOLLOUT)
                            stm32.write();
                    }
                    else if (auth.have(event.first))
                    {
                        if (event.second & EPOLLIN)
                            auth.read(event.first);
                        if (event.second & EPOLLOUT)
                            auth.write(event.first);
                    }
                    else if (clients.have(event.first))
                    {
                        if (event.second & EPOLLIN)
                            stm32.append(clients.read(event.first));
                        if (event.second & EPOLLOUT)
                            clients.write(event.first);
                    }
                }

                for (std::pair <int, uint32_t> const & event : events)
                {
                    static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                    if (event.second & err_mask)
                    {
                        try
                        {
                            epoll_wrap.unreg(event.first);
                        }
                        catch (...)
                        {}
                        
                        if (event.first == conn_sock.file_descriptor())
                            throw std::runtime_error("server socket error");
                        else if (event.first == tcp_server_sock.file_descriptor())
                            ;
                        else if (event.first == stm32.file_descriptor())
                            exit = 1;
                        else if (auth.have(event.first))
                            auth.remove(event.first);
                        else if (clients.have(event.first))
                            clients.unreg(event.first);
                    }
                }

                std::vector <int> acc = auth.check();
                for (int fd : acc)
                {
                    try
                    {
                        clients.reg(fd);
                    }
                    catch (...)
                    {
                        try
                        {
                            epoll_wrap.unreg(fd);
                        }
                        catch (...)
                        {}
                        close(fd);
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


