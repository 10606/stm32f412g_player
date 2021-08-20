#include "sender_data.h"

#include <string>

#include <unistd.h>
#include <stdexcept>

void sender_data_len::write ()
{
    if (!init)
        return;
    if (send_ptr == sizeof(len.value) + len.value)
        return;
    
    if (send_ptr < sizeof(len.value))
    {
        ssize_t wb = ::write(fd, len.bytes + send_ptr, sizeof(len.value) - send_ptr);
        if (wb == -1)
        {
            if ((errno != EINTR) && (errno != EPIPE))
                throw std::runtime_error("error write len");
        }
        else
        {
            send_ptr += wb;
        }
    }
    else
    {
        size_t cur_send_ptr = send_ptr - sizeof(len.value);
        ssize_t wb = ::write(fd, value.data() + cur_send_ptr, len.value - cur_send_ptr);
        if (wb == -1)
        {
            if ((errno != EINTR) && (errno != EPIPE))
                throw std::runtime_error("error write data");
        }
        else
        {
            send_ptr += wb;
        }
    }
}

