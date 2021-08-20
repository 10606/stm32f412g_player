#ifndef RECVER_DATA_H
#define RECVER_DATA_H

#include <string_view>
#include <stdint.h>
#include <stddef.h>

struct recver_data_len
{
    recver_data_len ():
        init(0),
        len{.value = 0},
        value(nullptr),
        recv_ptr(0),
        fd(-1)
    {}

    ~recver_data_len ()
    {
        delete [] value;
    }

    void set (int _fd)
    {
        delete [] value;
        
        init = _fd != -1;
        value = nullptr;
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
        return std::string_view(value, len.value);
    }
    
private:
    bool init;
    
    union
    {   
        uint32_t value;
        char bytes [sizeof(value)];
    } len;
    
    char * value;
    size_t recv_ptr;
    int fd;
};

#endif

