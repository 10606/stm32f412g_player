#ifndef UNIX_SERVER_SOCK_H
#define UNIX_SERVER_SOCK_H

#include <string>

struct unix_server_sock_t
{
    unix_server_sock_t (std::string const & sock_name, bool socket_activation, int _epoll_fd);
    ~unix_server_sock_t ();
    
    unix_server_sock_t (unix_server_sock_t const &) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t const &) = delete;
    unix_server_sock_t (unix_server_sock_t &&) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t &&) = delete;

    int accept ();
    
    int fd;
    int epoll_fd;
};

#endif

