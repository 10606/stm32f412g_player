#ifndef SENDER_DATA_H
#define SENDER_DATA_H

#include <string>
#include <stdint.h>
#include <stddef.h>
#include <stdexcept>

template <typename Socket>
struct sender_data_len
{
    sender_data_len ():
        init(0),
        len(0),
        value(),
        send_ptr(0),
        sock(nullptr)
    {}
    
    void swap (sender_data_len & rhs) noexcept
    {
        std::swap(init, rhs.init);
        std::swap(len, rhs.len);
        std::swap(value, rhs.value);
        std::swap(send_ptr, rhs.send_ptr);
        std::swap(sock, rhs.sock);
    }
    
    sender_data_len (sender_data_len const &) = delete;
    sender_data_len (sender_data_len && rhs) noexcept :
        sender_data_len()
    {
        swap(rhs);
    }
    
    sender_data_len & operator = (sender_data_len const &) = delete;
    sender_data_len & operator = (sender_data_len && rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
    
    void set (Socket & _sock, std::string && _value)
    {
        value = std::move(_value);
        len = value.size();
        send_ptr = 0;
        sock = &_sock;
        init = sock->fd() != -1;
    }
    
    void reset () noexcept
    {
        value = std::string();
        len = 0;
        send_ptr = 0;
        sock = nullptr;
        init = 0;
    }
    
    void write ();
    
    bool ready () const noexcept
    {
        return init && (send_ptr == sizeof(len) + len);
    }
    
private:
    bool init;
    
    uint32_t len;
    std::string value;
    size_t send_ptr;
    Socket * sock;
};

template <typename Socket>
void sender_data_len <Socket> ::write ()
{
    if (!init)
        return;
    if (send_ptr == sizeof(len) + len)
        return;
    
    if (send_ptr < sizeof(len))
    {
        ssize_t wb = sock->write(reinterpret_cast <char *> (&len) + send_ptr, sizeof(len) - send_ptr);
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
        size_t cur_send_ptr = send_ptr - sizeof(len);
        ssize_t wb = sock->write(value.data() + cur_send_ptr, len - cur_send_ptr);
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


#endif

