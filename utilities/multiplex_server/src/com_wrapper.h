#ifndef COM_WRAPPER_H
#define COM_WRAPPER_H

#include <stdint.h>
#include <string>
#include <string_view>

#include "ring_buffer.h"

struct com_wrapper_t
{
    com_wrapper_t (char const * _file_name, int _epoll_fd);
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
    int epoll_fd; // not owned
};

#endif

