#include "epoll_reg.h"

#include <sys/epoll.h>
#include <cstddef>
#include <errno.h>
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

