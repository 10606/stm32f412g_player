#ifndef RECVER_DATA_H
#define RECVER_DATA_H

#include <string_view>
#include <stdint.h>
#include <stddef.h>
#include <algorithm>
#include <memory>

template <typename Socket>
struct recver_data_len
{
    recver_data_len ():
        init(0),
        len{.value = 0},
        value(),
        recv_ptr(0),
        sock(nullptr)
    {}

    ~recver_data_len () = default;
    
    void swap (recver_data_len & rhs) noexcept
    {
        std::swap(init, rhs.init);
        std::swap(len, rhs.len);
        std::swap(value, rhs.value);
        std::swap(recv_ptr, rhs.recv_ptr);
        std::swap(sock, rhs.sock);
    }
    
    recver_data_len (recver_data_len const &) = delete;
    recver_data_len (recver_data_len && rhs) noexcept :
        recver_data_len()
    {
        swap(rhs);
    }
    
    recver_data_len & operator = (recver_data_len const &) = delete;
    recver_data_len & operator = (recver_data_len && rhs) noexcept
    {
        swap(rhs);
        return *this;
    }

    void set (Socket & _sock)
    {
        init = _sock.fd() != -1;
        value.reset();
        recv_ptr = 0;
        sock = &_sock;
    }
    
    void reset () noexcept
    {
        init = 0;
        value.reset();
        recv_ptr = 0;
        sock = nullptr;
    }
    
    void read ();
    
    bool ready ()
    {
        return init && (recv_ptr == sizeof(len.value) + len.value);
    }
    
    std::string_view get ()
    {
        return std::string_view(value.get(), len.value);
    }
    
private:
    bool init;
    
    union
    {   
        uint32_t value;
        char bytes [sizeof(value)];
    } len;
    
    std::unique_ptr <char []> value;
    size_t recv_ptr;
    Socket * sock;
};

template <typename Socket>
void recver_data_len <Socket> ::read ()
{
    if (!init)
        return;
    if (recv_ptr == sizeof(len.value) + len.value)
        return;
    
    if (recv_ptr < sizeof(len.value))
    {
        ssize_t rb = sock->read(len.bytes + recv_ptr, sizeof(len.value) - recv_ptr);
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
        ssize_t rb = sock->read(value.get() + cur_recv_ptr, len.value - cur_recv_ptr);
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


#endif

