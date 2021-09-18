#include "epoll_wrapper.h"

#include <functional>
#include <cstddef>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

void epoll_reg (int epoll_fd, int fd, uint32_t flag)
{
    flag |= EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    epoll_data_t ep_data;
    ep_data.fd = fd;

    epoll_event ep_event = {flag, ep_data};
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ep_event);
    if (ret == -1 && errno == ENOENT)
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ep_event);
    
    if (ret == -1)
        throw std::runtime_error("can't add to epoll");
}

void epoll_del (int epoll_fd, int fd)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("can't del from epoll");
}



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
    epoll_reg(epoll_fd, fd, flag);
}

void epoll_wraper::unreg (int fd)
{
    epoll_del(epoll_fd, fd);
}

std::vector <epoll_wraper::e_event> epoll_wraper::wait (int timeout)
{
    epoll_event events[100];
    std::vector <e_event> ans;
    int ret = epoll_wait(epoll_fd, events, std::extent <decltype(events)> :: value, timeout);
    if (ret == -1)
    {
        if (errno != EINTR)
            throw std::runtime_error("error while epoll_wait");
        else
            return ans;
    }
    ans.reserve(ret);
    for (int i = 0; i != ret; ++i)
    {
        ans.emplace_back(static_cast <uint32_t> (events[i].data.fd), 
                         static_cast <uint32_t> (events[i].events));
    }
    
    return ans;
}


