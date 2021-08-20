#include "client_wrapper.h"

#include "epoll_reg.h"
#include "usb_commands.h"
#include <sys/epoll.h>
#include <stdexcept>
#include <limits>
#include <unistd.h>
#include <string.h>

clients_wrapper_t::clients_wrapper_t (int _epoll_fd) :
    epoll_fd(_epoll_fd),
    buffer(nullptr),
    size(0),
    capacity(0),
    last_full_struct(0),
    pointers(),
    add_offset(0),
    less_size(),
    not_registred()
{}

clients_wrapper_t::~clients_wrapper_t ()
{
    for (auto const & i : pointers)
    {
        epoll_del(epoll_fd, i.first);
        close(i.first);
    }
    delete [] buffer;
}

size_t inc_struct_ptr (char c) noexcept
{
    switch (c)
    {
    case cur_song_info:
        return sizeof(cur_song_info_t);

    case displayed_song_info:
        return sizeof(displayed_song_info_t);

    case pl_list_info:
        return sizeof(pl_list_info_t);

    case volume_info:
        return sizeof(volume_info_t);

    case state_info:
        return sizeof(state_info_t);

    default:
        return 1;
    }
}

void clients_wrapper_t::reg (int fd)
{
    if (fd == -1)
        throw std::runtime_error("bad fd");
    
    if (pointers.size() >= max_clients)
    {
        close(fd);
        return;
    }
    
    pointers.insert({fd, last_full_struct + add_offset});
    less_size.insert({last_full_struct + add_offset, fd});
    if (last_full_struct != size)
    {
        epoll_reg(epoll_fd, fd, (EPOLLIN | EPOLLOUT));
    }
    else
    {
        not_registred.insert(fd);
        epoll_reg(epoll_fd, fd, EPOLLIN);
    }
}

void clients_wrapper_t::unreg (int fd)
{
    std::map <int, size_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("unreg unknown fd");
    less_size.erase({it->second, it->first});
    not_registred.erase(it->first);
    pointers.erase(it);
    epoll_del(epoll_fd, fd);
    close(fd);
    shrink_to_fit();
}

void clients_wrapper_t::realloc (size_t start_index, size_t needed)
{
    size_t new_size = size - start_index + delta_capacity + needed;
    char * new_buffer = new char [new_size];
    capacity = new_size;
    size -= start_index;
    last_full_struct -= start_index;
    if (buffer)
    {
        memmove(new_buffer, buffer + start_index, size);
        delete [] buffer;
    }
    buffer = new_buffer;
 
    add_offset += start_index;
    if (add_offset > std::numeric_limits <decltype(add_offset)> :: max() / 2)
    {
        std::set <std::pair <size_t, int> > new_less_size;
        for (auto const & v : less_size)
            new_less_size.insert({v.first - add_offset, v.second});
        less_size.swap(new_less_size);
        
        for (auto & v : pointers)
            v.second -= add_offset;
        add_offset = 0;
    }
}

void clients_wrapper_t::shrink_to_fit ()
{
    if (capacity > max_capacity)
    {
        while (!less_size.empty())
        {
            std::set <std::pair <size_t, int> > :: iterator it = less_size.begin();
            if (it->first - add_offset + max_capacity / 2 < size)
                unreg(it->second);
            else
                break;
        }
    }
    
    size_t min_ptr = last_full_struct;
    if (!less_size.empty())
        min_ptr = std::min(min_ptr, less_size.begin()->first - add_offset);
        
    if (min_ptr < delta_capacity)
        return; // useless
    
    realloc(min_ptr, 0);
}

void clients_wrapper_t::write (int fd)
{
    std::map <int, size_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("write unknown fd");
    size_t offset = it->second - add_offset;
    ssize_t wb = ::write(fd, buffer + offset, size - offset);
    if (wb < 0)
    {
        if ((errno != EINTR) && (errno != EPIPE))
            throw std::runtime_error("error write");
        else
            wb = 0;
    }
    less_size.erase({it->second, it->first});
    it->second += wb;
    less_size.insert({it->second, it->first});
    if (it->second - add_offset == size)
    {
        epoll_reg(epoll_fd, fd, EPOLLIN);
        not_registred.insert(it->first);
    }
    shrink_to_fit();
}

std::string clients_wrapper_t::read (int fd)
{
    std::map <int, size_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("read for unknown fd");
    
    char buff [64];
    ssize_t rb = ::read(fd, buff, sizeof(buff));
    if (rb < 0)
    {
        if ((errno != EINTR) && (errno != EPIPE))
            throw std::runtime_error("error read");
        else
            return std::string();
    }
    return std::string(buff, rb);
}

void clients_wrapper_t::append (std::string_view value)
{
    if (size + value.size() > capacity)
        realloc(0, value.size());
    size += value.copy(buffer + size, capacity - size);
    
    size_t new_last_full_struct = last_full_struct;
    while (new_last_full_struct < size)
    {
        last_full_struct = new_last_full_struct;
        new_last_full_struct += inc_struct_ptr(buffer[last_full_struct]);
    }
    if (new_last_full_struct == size)
        last_full_struct = size;
    
    for (int const & i : not_registred)
        epoll_reg(epoll_fd, i, EPOLLIN | EPOLLOUT);
    not_registred.clear();

    shrink_to_fit();
}

bool clients_wrapper_t::have (int fd) const noexcept
{
    std::map <int, size_t> :: const_iterator it = pointers.find(fd);
    return (it != pointers.end());
}

