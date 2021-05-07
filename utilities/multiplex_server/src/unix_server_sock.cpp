#include "unix_server_sock.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <sys/un.h>
#include <systemd/sd-daemon.h>
#include "epoll_reg.h"

unix_server_sock_t::unix_server_sock_t (std::string const & sock_name, bool socket_activation, int _epoll_fd) :
    fd(-1),
    epoll_fd(_epoll_fd)
{
    if (!socket_activation)
    {
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == -1)
            throw std::runtime_error("create server socket");
        
        struct sockaddr_un sa_un; 
        memset(&sa_un, 0, sizeof(struct sockaddr_un));
        sa_un.sun_family = AF_UNIX;
        strncpy(sa_un.sun_path, sock_name.c_str(), sizeof(sa_un.sun_path) - 1);
        
        unlink(sock_name.c_str());
        if (bind(fd, reinterpret_cast <sockaddr *> (&sa_un), sizeof(sa_un)) == -1)
        {
            close(fd);
            throw std::runtime_error("bind server socket");
        }
        if (listen(fd, 10) == -1)
        {
            unlink(sock_name.c_str());
            close(fd);
            throw std::runtime_error("listen server socket");
        }
    }
    else
    {
        if (sd_listen_fds(0) != 1)
            throw std::runtime_error("wrong amount of listen sockets");
        fd = SD_LISTEN_FDS_START + 0;
    }
    
    if (epoll_reg(epoll_fd, fd, EPOLLIN) == -1)
    {
        unlink(sock_name.c_str());
        close(fd);
        throw std::runtime_error("can't add to epoll");
    }
}

unix_server_sock_t::~unix_server_sock_t ()
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

