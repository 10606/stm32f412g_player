#ifndef COM_WRAPPER_H
#define COM_WRAPPER_H

#include <stdint.h>
#include <string>
#include <string_view>

struct com_wrapper_t
{
    com_wrapper_t (std::string const & _file_name, int _epoll_fd);
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
    void realloc (size_t needed);
    void shrink_to_fit ();
    
    int fd;
    static size_t const delta_capacity = 128;
    char * buffer;
    size_t pos;
    size_t size;
    size_t capacity;
    int epoll_fd; // not owned
};

#endif

