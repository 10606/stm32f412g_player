#include "recver_data.h"

#include <unistd.h>
#include <stdexcept>

void recver_data_len::read ()
{
    if (!init)
        return;
    if (recv_ptr == sizeof(len.value) + len.value)
        return;
    
    if (recv_ptr < sizeof(len.value))
    {
        ssize_t rb = ::read(fd, len.bytes + recv_ptr, sizeof(len.value) - recv_ptr);
        if (rb == -1)
        {
            if ((errno != EINTR) && (errno != EPIPE))
                throw std::runtime_error("error read len");
        }
        else
        {
            recv_ptr += rb;
            len.value += 0;
            if (recv_ptr == sizeof(len.value))
            {
                value = std::make_unique <char []> (len.value);
            }
        }
    }
    else
    {
        size_t cur_recv_ptr = recv_ptr - sizeof(len.value);
        ssize_t rb = ::read(fd, value.get() + cur_recv_ptr, len.value - cur_recv_ptr);
        if (rb == -1)
        {
            if ((errno != EINTR) && (errno != EPIPE))
                throw std::runtime_error("error read len");
        }
        else
        {
            recv_ptr += rb;
        }
    }
}

