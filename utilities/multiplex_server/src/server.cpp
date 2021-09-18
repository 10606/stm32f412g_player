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
        unix_server_sock_t conn_sock(sock_name, socket_activation, epoll_wrap);
        tcp_server_sock_t tcp_server_sock(INADDR_ANY, 750, epoll_wrap);
            
        try
        {
            clients_wrapper_t clients(epoll_wrap);
            com_wrapper_t stm32(stm32_name, epoll_wrap);
            authentificator_t auth(epoll_wrap);

            bool exit = 0;
            while (!exit)
            {
                clients.remove_too_big();
                std::vector <epoll_wraper::e_event> events = epoll_wrap.wait();
                for (epoll_wraper::e_event const & event : events)
                {
                    if (event.fd == conn_sock.file_descriptor())
                    {
                        if (event.mask & EPOLLIN)
                            clients.reg(conn_sock.accept());
                    }
                    else if (event.fd == tcp_server_sock.file_descriptor())
                    {
                        try
                        {
                            if (event.mask & EPOLLIN)
                                auth.add(tcp_server_sock.accept());
                        }
                        catch (...)
                        {}
                    }
                    else if (event.fd == stm32.file_descriptor())
                    {
                        if (event.mask & EPOLLIN)
                            clients.append(stm32.read());
                        if (event.mask & EPOLLOUT)
                            stm32.write();
                    }
                    else if (auth.have(event.fd))
                    {
                        if (event.mask & EPOLLIN)
                            auth.read(event.fd);
                        if (event.mask & EPOLLOUT)
                            auth.write(event.fd);
                    }
                    else if (clients.have(event.fd))
                    {
                        if (event.mask & EPOLLIN)
                            stm32.append(clients.read(event.fd));
                        if (event.mask & EPOLLOUT)
                            clients.write(event.fd);
                    }
                }

                for (epoll_wraper::e_event const & event : events)
                {
                    static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                    if (event.mask & err_mask)
                    {
                        if (event.fd == conn_sock.file_descriptor())
                            throw std::runtime_error("server socket error");
                        else if (event.fd == tcp_server_sock.file_descriptor())
                            tcp_server_sock.close();
                        else if (event.fd == stm32.file_descriptor())
                            exit = 1;
                        else if (auth.have(event.fd))
                            auth.remove(event.fd);
                        else if (clients.have(event.fd))
                            clients.unreg(event.fd);
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
                    std::vector <epoll_wraper::e_event> events = epoll_wrap.wait(1000);
                    for (epoll_wraper::e_event const & event : events)
                    {
                        if ((event.fd == conn_sock.file_descriptor()) &&
                            (event.mask & EPOLLIN))
                        {
                            flag = 1;
                            int fd = conn_sock.accept();
                            if (fd != -1)
                                close(fd);
                        }
                        else if ((event.fd == tcp_server_sock.file_descriptor()) &&
                            (event.mask & EPOLLIN))
                        {
                            flag = 1;
                            int fd = tcp_server_sock.accept();
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


