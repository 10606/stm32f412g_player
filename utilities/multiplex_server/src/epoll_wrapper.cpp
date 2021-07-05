#include "epoll_wrapper.h"

#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include "epoll_reg.h"

epoll_wraper::epoll_wraper () :
    epoll_fd(-1)
{
    epoll_fd = epoll_create(4);
    if (epoll_fd < 0)
        throw std::runtime_error("can't create epoll");
}

epoll_wraper::~epoll_wraper ()
{
    close(epoll_fd);
}

void epoll_wraper::reg (int fd, uint32_t flag)
{
    int ret = epoll_reg(epoll_fd, fd, flag);
    if (ret == -1)
        throw std::runtime_error("can't reg in epoll");
}

void epoll_wraper::unreg (int fd)
{
    int ret = epoll_del(epoll_fd, fd);
    if (ret == -1)
        throw std::runtime_error("can't del from epoll");
}

std::vector <std::pair <int, uint32_t> > epoll_wraper::wait ()
{
    epoll_event events[100];
    int ret = epoll_wait(epoll_fd, events, std::extent <decltype(events)> :: value, -1);
    if (ret == -1)
    {
        if (errno != EINTR)
            throw std::runtime_error("error while epoll_wait");
        else
            return {};
    }
    std::vector <std::pair <int, uint32_t> > ans;
    ans.reserve(ret);
    for (int i = 0; i != ret; ++i)
    {
        ans.emplace_back(static_cast <uint32_t> (events[i].data.fd), 
                         static_cast <uint32_t> (events[i].events));
    }
    
    return ans;
}


