#ifndef UNIX_SERVER_SOCK_H
#define UNIX_SERVER_SOCK_H

#include <string>
#include <stdexcept>
#include <sys/socket.h>

#include "epoll_wrapper.h"

struct unix_server_sock_t
{
    unix_server_sock_t (char const * sock_name, bool socket_activation, epoll_wraper & _epoll);
    ~unix_server_sock_t ();
    
    unix_server_sock_t (unix_server_sock_t const &) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t const &) = delete;
    unix_server_sock_t (unix_server_sock_t &&) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t &&) = delete;

    int accept ();
    
    int file_descriptor () const noexcept
    {
        return fd;
    }
    
private:
    int fd;
    epoll_wraper & epoll;
};

#endif

