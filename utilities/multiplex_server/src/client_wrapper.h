#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

#include <sys/epoll.h>
#include <map>
#include <set>
#include <string>
#include <string_view>

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
    void realloc (size_t start_index, size_t needed);
    void shrink_to_fit ();
    void write (int fd);
    std::string read (int fd);
    void append (std::string_view value);
    
private:
    static size_t const delta_capacity = 1024;
    int epoll_fd; // not owned
    char * buffer;
    size_t size;
    size_t capacity;
    size_t last_full_struct;
    
    std::map <int, size_t> pointers; // fd -> offset
    size_t add_offset; // offset buffer in history
    std::set <std::pair <size_t, int> > less_size; // <offset, fd>
    std::set <int> not_registred; // not registred fd on write in epoll
};

#endif

