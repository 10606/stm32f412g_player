#ifndef READ_WRITE_H
#define READ_WRITE_H

#include <unistd.h>
#include <errno.h>
#include <stdexcept>
#include <stddef.h>

inline size_t read_ign (int fd, void * buf, size_t count)
{
    if (count == 0)
        return 0;
    ssize_t ret = ::read(fd, buf, count);
    if (ret < 0)
    {
        if ((errno != EINTR) && 
            (errno != ECONNABORTED) && 
            (errno != ECONNRESET))
            throw std::runtime_error("error read");
        return 0;
    }
    return ret;
}

inline size_t write_ign (int fd, void const * buf, size_t count)
{
    if (count == 0)
        return 0;
    ssize_t ret = ::write(fd, buf, count);
    if (ret < 0)
    {
        if ((errno != EINTR) && 
            (errno != EPIPE) && 
            (errno != ECONNABORTED) && 
            (errno != ECONNRESET))
            throw std::runtime_error("error write");
        return 0;
    }
    return ret;
}

#endif

