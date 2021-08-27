#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

#include <sys/epoll.h>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include "ring_buffer.h"

struct clients_wrapper_t
{
    clients_wrapper_t (int _epoll_fd);
    ~clients_wrapper_t ();
    
    clients_wrapper_t (clients_wrapper_t const &) = delete;
    clients_wrapper_t & operator = (clients_wrapper_t const &) = delete;
    clients_wrapper_t (clients_wrapper_t &&) = delete;
    clients_wrapper_t & operator = (clients_wrapper_t &&) = delete;
    
    void reg (int fd);
    void unreg (int fd);
    void write (int fd);
    std::string read (int fd);
    void append (std::string_view value);
    void remove_too_big ();
    
    bool have (int fd) const noexcept
    {
        std::map <int, size_t> :: const_iterator it = pointers.find(fd);
        return (it != pointers.end());
    }

private:
    void shrink_to_fit ();
    
    static size_t const delta_capacity = 1024;
    static size_t const max_capacity = 4 * 1024 * 1024;
    static size_t const max_clients = 1000;
    
    int epoll_fd; // not owned
    ring_buffer data;
    size_t last_full_struct_ptr;
    
    std::map <int, size_t> pointers; // fd -> offset
    std::set <std::pair <size_t, int> > less_size; // <offset, fd>
    std::set <int> not_registred; // not registred fd on write in epoll
};

#endif

