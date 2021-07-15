#ifndef EPOLL_WRAPPER_H
#define EPOLL_WRAPPER_H

#include <vector>
#include <stdint.h>

struct epoll_wraper
{
    epoll_wraper ();
    ~epoll_wraper ();
    
    epoll_wraper (epoll_wraper const &) = delete;
    epoll_wraper & operator = (epoll_wraper const &) = delete;
    epoll_wraper (epoll_wraper &&) = delete;
    epoll_wraper & operator = (epoll_wraper &&) = delete;
    
    void reg (int fd, uint32_t flag);
    void unreg (int fd);
    std::vector <std::pair <int, uint32_t> > wait (int timeout = -1);
    
    int fd () const noexcept
    {
        return epoll_fd;
    }
    
private:
    int epoll_fd;
};

#endif

