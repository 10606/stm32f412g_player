#ifndef TCP_SERVER_SOCK_H
#define TCP_SERVER_SOCK_H

#include <stddef.h>
#include <stdint.h>

#include "epoll_wrapper.h"

struct tcp_server_sock_t
{
    // INADDR_ANY, 750
    tcp_server_sock_t (uint32_t _addr, uint16_t _port, epoll_wraper & _epoll) noexcept;
    ~tcp_server_sock_t ();
    
    tcp_server_sock_t (tcp_server_sock_t const &) = delete;
    tcp_server_sock_t & operator = (tcp_server_sock_t const &) = delete;
    tcp_server_sock_t (tcp_server_sock_t &&) = delete;
    tcp_server_sock_t & operator = (tcp_server_sock_t &&) = delete;
    
    int accept ();
    
    int file_descriptor () const noexcept
    {
        return fd;
    }
    
    void close () noexcept;
    
private:
    int fd;
    epoll_wraper & epoll;
};

#endif

