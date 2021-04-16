#ifndef EPOLL_WRAPPER_H
#define EPOLL_WRAPPER_H

#include <vector>
#include <stdint.h>

struct epoll_wraper
{
    epoll_wraper ();
    ~epoll_wraper ();
    
    void reg (int fd, uint32_t flag);
    void unreg (int fd);
    std::vector <std::pair <int, uint32_t> > wait ();
    
    int epoll_fd;
};

#endif

