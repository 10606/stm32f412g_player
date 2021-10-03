#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

#include <sys/epoll.h>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include "ring_buffer.h"
#include "epoll_wrapper.h"

struct clients_wrapper_t
{
    clients_wrapper_t (epoll_wraper & _epoll);
    ~clients_wrapper_t ();
    
    clients_wrapper_t (clients_wrapper_t const &) = delete;
    clients_wrapper_t & operator = (clients_wrapper_t const &) = delete;
    clients_wrapper_t (clients_wrapper_t &&) = delete;
    clients_wrapper_t & operator = (clients_wrapper_t &&) = delete;
    
    template <typename Socket>
    void reg (int fd);
    
    void unreg (int fd) noexcept;
    void write (int fd);
    std::string read (int fd);
    void append (std::string_view value);
    void remove_too_big ();
    
    bool have (int fd) const noexcept
    {
        std::map <int, client_data_t> :: const_iterator it = pointers.find(fd);
        return (it != pointers.end());
    }

private:
    void shrink_to_fit ();
    
    static size_t const delta_capacity = 1024;
    static size_t const max_capacity = 4 * 1024 * 1024;
    static size_t const max_clients = 1000;
    
    epoll_wraper & epoll;
    ring_buffer data;
    size_t last_full_struct_ptr;
    
    struct client_pos
    {
        size_t offset;
        int fd;
        
        friend std::strong_ordering operator <=> (client_pos const & lhs, client_pos const & rhs) = default;
    };
    
    struct socket_base_t
    {
        virtual ~socket_base_t () = default;
        virtual ssize_t read (char * buffer, size_t size) = 0;
        virtual ssize_t write (char const * data, size_t size) = 0;
    };
    
    template <typename Socket>
    struct socket_wrap_t : socket_base_t
    {
        socket_wrap_t (int fd) : 
            value(fd)
        {}
        
        virtual ~socket_wrap_t () = default;
        
        virtual ssize_t read (char * buffer, size_t size)
        {
            return value.read(buffer, size);
        }
        
        virtual ssize_t write (char const * data, size_t size)
        {
            return value.write(data, size);
        }
        
    private:
        Socket value; 
    };
    
    struct client_data_t
    {
        std::unique_ptr <socket_base_t> socket;
        size_t pointer;
    };
    
    std::map <int, client_data_t> pointers; // fd -> socket, offset
    std::set <client_pos> less_size; // <offset, fd>
    std::set <int> not_registred; // not registred fd on write in epoll
};


template <typename Socket>
void clients_wrapper_t::reg (int fd)
{
    if (fd == -1)
        throw std::runtime_error("bad fd");
    
    if (pointers.size() >= max_clients)
    {
        close(fd);
        return;
    }
    
    pointers.insert(std::pair <int, client_data_t> {
        fd, 
        client_data_t{std::make_unique <socket_wrap_t <Socket> > (fd), last_full_struct_ptr}
    });
    
    less_size.insert({last_full_struct_ptr, fd});
    if (last_full_struct_ptr != data.end())
    {
        epoll.reg(fd, (EPOLLIN | EPOLLOUT));
    }
    else
    {
        not_registred.insert(fd);
        epoll.reg(fd, EPOLLIN);
    }
}


#endif

