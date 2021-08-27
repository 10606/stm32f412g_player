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
    data(),
    last_full_struct_ptr(0),
    pointers(),
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
    
    pointers.insert({fd, last_full_struct_ptr});
    less_size.insert({last_full_struct_ptr, fd});
    if (last_full_struct_ptr != data.end())
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

void clients_wrapper_t::remove_too_big ()
{
    if (data.size() > max_capacity)
    {
        while (!less_size.empty())
        {
            std::set <std::pair <size_t, int> > :: iterator it = less_size.begin();
            if (it->first + max_capacity / 2 < data.end())
                unreg(it->second);
            else
                break;
        }
    }
    shrink_to_fit();
}

void clients_wrapper_t::shrink_to_fit ()
{
    size_t min_ptr = last_full_struct_ptr;
    if (!less_size.empty())
        min_ptr = std::min(min_ptr, less_size.begin()->first);
        
    data.erase(min_ptr - data.begin());
}

void clients_wrapper_t::write (int fd)
{
    std::map <int, size_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("write unknown fd");
    size_t wb = data.write(fd, it->second);
    less_size.erase({it->second, it->first});
    it->second += wb;
    less_size.insert({it->second, it->first});
    if (it->second == data.end())
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
    size_t diff = data.add(value);
    if (diff != 0)
    {
        std::set <std::pair <size_t, int> > new_less_size;
        for (auto const & v : less_size)
            new_less_size.insert({v.first - diff, v.second});
        less_size.swap(new_less_size);
        
        for (auto & v : pointers)
            v.second -= diff;
        last_full_struct_ptr -= diff;
    }
    
    size_t new_last_full_struct = last_full_struct_ptr;
    while (new_last_full_struct < data.end())
    {
        last_full_struct_ptr = new_last_full_struct;
        new_last_full_struct += inc_struct_ptr(data[last_full_struct_ptr]);
    }
    if (new_last_full_struct == data.end())
        last_full_struct_ptr = data.end();
    
    for (int const & i : not_registred)
        epoll_reg(epoll_fd, i, EPOLLIN | EPOLLOUT);
    not_registred.clear();
}

