#ifndef UNIX_SERVER_SOCK_H
#define UNIX_SERVER_SOCK_H

#include <string>

struct unix_server_sock_t
{
    unix_server_sock_t (std::string const & _sock_name, int _epoll_fd);
    ~unix_server_sock_t ();

    int accept ();
    
    int fd;
    int epoll_fd;
    std::string sock_name;
};

#endif

