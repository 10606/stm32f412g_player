#ifndef SENDER_DATA_H
#define SENDER_DATA_H

#include <string>
#include <stdint.h>
#include <stddef.h>

struct sender_data_len
{
    sender_data_len ():
        init(0),
        len{.value = 0},
        value(),
        send_ptr(0),
        fd(-1)
    {}
    
    void swap (sender_data_len & rhs) noexcept
    {
        std::swap(init, rhs.init);
        std::swap(len, rhs.len);
        std::swap(value, rhs.value);
        std::swap(send_ptr, rhs.send_ptr);
        std::swap(fd, rhs.fd);
    }
    
    sender_data_len (sender_data_len const &) = delete;
    sender_data_len (sender_data_len && rhs) noexcept :
        sender_data_len()
    {
        swap(rhs);
    }
    
    sender_data_len & operator = (sender_data_len const &) = delete;
    sender_data_len & operator =(sender_data_len && rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
    
    void set (int _fd, std::string && _value)
    {
        value = std::move(_value);
        len.value = value.size();
        send_ptr = 0;
        fd = _fd;
        init = fd != -1;
    }
    
    void write ();
    
    bool ready () const noexcept
    {
        return init && (send_ptr == sizeof(len.value) + len.value);
    }
    
private:
    bool init;
    
    union 
    {
        uint32_t value;
        char bytes [sizeof(value)];
    } len;
    
    std::string value;
    size_t send_ptr;
    int fd;
};

#endif

