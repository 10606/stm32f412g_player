#ifndef UNIX_SERVER_SOCK_H
#define UNIX_SERVER_SOCK_H

#include <string>
#include <sys/socket.h>

struct unix_server_sock_t
{
    unix_server_sock_t (std::string const & sock_name, bool socket_activation, int _epoll_fd);
    ~unix_server_sock_t ();
    
    unix_server_sock_t (unix_server_sock_t const &) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t const &) = delete;
    unix_server_sock_t (unix_server_sock_t &&) = delete;
    unix_server_sock_t & operator = (unix_server_sock_t &&) = delete;

    int accept () noexcept
    {
        return ::accept(fd, NULL, NULL);
    }
    
    int file_descriptor () const noexcept
    {
        return fd;
    }
    
private:
    int fd;
    int epoll_fd;
};

#endif

