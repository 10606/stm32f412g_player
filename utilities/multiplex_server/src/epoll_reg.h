#ifndef EPOLL_REG_H
#define EPOLL_REG_H

#include <stdint.h>

void epoll_reg (int epoll_fd, int fd, uint32_t flag);
void epoll_del (int epoll_fd, int fd);

#endif

