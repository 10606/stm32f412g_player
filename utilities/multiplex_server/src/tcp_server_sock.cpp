#include "tcp_server_sock.h"

#include "epoll_wrapper.h"

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <stdexcept>

tcp_server_sock_t::tcp_server_sock_t (uint32_t _addr, uint16_t _port, epoll_wraper & _epoll) noexcept :
    fd(-1),
    epoll(_epoll)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
        ;
    
    sockaddr_in addr = {AF_INET, htons(_port), {htonl(_addr)}};
    if (bind(fd, reinterpret_cast <sockaddr *> (&addr), sizeof(addr)) == -1)
    {
        ::close(fd);
        perror("bind");
        fd = -1;
    }
    
    if (listen(fd, 10) == -1)
    {
        ::close(fd);
        perror("listen");
        fd = -1;
    }
    
    try
    {
        epoll.reg(fd, EPOLLIN);
    }
    catch (std::exception & e)
    {
        ::close(fd);
        std::cerr << e.what() << '\n';
        fd = -1;
    }
}

tcp_server_sock_t::~tcp_server_sock_t ()
{
    close();
}

void tcp_server_sock_t::close () noexcept
{
    if (fd != -1)
    {
        epoll.unreg(fd);
        ::close(fd);
        fd = -1;
    }
}

int tcp_server_sock_t::accept ()
{
    if (fd == -1)
        throw std::runtime_error("bad fd");
    
    while (1)
    {
        int ret = ::accept(fd, NULL, NULL);
        if (ret == -1)
        {
            if (errno != EINTR)
                throw std::runtime_error("error on accept");
        }
        else
        {
            return ret;
        }
    }
}


