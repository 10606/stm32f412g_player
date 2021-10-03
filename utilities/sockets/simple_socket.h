#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

#include <stddef.h>
#include <unistd.h>

struct socket_t
{
    socket_t (int _fd) :
        fd_v(_fd)
    {}

    ssize_t read (char * buffer, size_t size)
    {
        return ::read(fd_v, buffer, size);
    }

    ssize_t write (char const * data, size_t size)
    {
        return ::write(fd_v, data, size);
    }

    int fd () const noexcept
    {
        return fd_v;
    }

private:
    int fd_v;
};


#endif

