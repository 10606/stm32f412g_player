#ifndef EPOLL_REG_H
#define EPOLL_REG_H

#include <stdint.h>

int epoll_reg (int epoll_fd, int fd, uint32_t flag);

#endif

