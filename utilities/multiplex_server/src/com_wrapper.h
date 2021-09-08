#ifndef COM_WRAPPER_H
#define COM_WRAPPER_H

#include <stdint.h>
#include <string>
#include <string_view>

#include "ring_buffer.h"
#include "epoll_wrapper.h"

struct com_wrapper_t
{
    com_wrapper_t (char const * _file_name, epoll_wraper & _epoll);
    ~com_wrapper_t ();
    
    com_wrapper_t (com_wrapper_t const &) = delete;
    com_wrapper_t & operator = (com_wrapper_t const &) = delete;
    com_wrapper_t (com_wrapper_t &&) = delete;
    com_wrapper_t & operator = (com_wrapper_t &&) = delete;
    
    void write ();
    void append (std::string_view value);
    std::string read ();
    
    int file_descriptor () const noexcept
    {
        return fd;
    }
    
private:
    int fd;
    ring_buffer data;
    size_t pos;
    epoll_wraper & epoll;
};

#endif

