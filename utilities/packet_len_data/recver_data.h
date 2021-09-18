#ifndef RECVER_DATA_H
#define RECVER_DATA_H

#include <string_view>
#include <stdint.h>
#include <stddef.h>
#include <algorithm>
#include <memory>

struct recver_data_len
{
    recver_data_len ():
        init(0),
        len{.value = 0},
        value(),
        recv_ptr(0),
        fd(-1)
    {}

    ~recver_data_len () = default;
    
    void swap (recver_data_len & rhs) noexcept
    {
        std::swap(init, rhs.init);
        std::swap(len, rhs.len);
        std::swap(value, rhs.value);
        std::swap(recv_ptr, rhs.recv_ptr);
        std::swap(fd, rhs.fd);
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

    void set (int _fd)
    {
        init = _fd != -1;
        value.reset();
        recv_ptr = 0;
        fd = _fd;
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
    int fd;
};

#endif

