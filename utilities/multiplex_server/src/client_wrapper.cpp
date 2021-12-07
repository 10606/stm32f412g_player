#include "client_wrapper.h"

#include "usb_commands.h"
#include "epoll_wrapper.h"
#include "playlist_structures.h"

#include <stdexcept>
#include <limits>
#include <unistd.h>
#include <string.h>

clients_wrapper_t::clients_wrapper_t (epoll_wraper & _epoll) :
    epoll(_epoll),
    data(2048),
    last_full_struct_ptr(0),
    pointers(),
    less_size(),
    not_registred()
{}

clients_wrapper_t::~clients_wrapper_t ()
{
    std::for_each(pointers.begin(), pointers.end(),
        [this] (decltype(pointers)::value_type const & i) -> void
        {
            epoll.unreg(i.first);
            close(i.first);
        });
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

size_t inc_command_ptr (char c) noexcept
{
    return calc_need_rd(c);
}

void clients_wrapper_t::unreg (int fd) noexcept
{
    try
    {
        pointers_it_t it = pointers.find(fd);
        if (it == pointers.end())
            return;
        less_size.erase({it->second.pointer, it->first});
        not_registred.erase(it->first);
        pointers.erase(it);
        epoll.unreg(fd);
        close(fd);
        shrink_to_fit();
    }
    catch (...)
    {}
}

void clients_wrapper_t::remove_too_big ()
{
    if (data.size() > max_capacity)
    {
        while (!less_size.empty())
        {
            std::set <client_pos> :: iterator it = less_size.begin();
            if (it->offset + max_capacity / 2 < data.end())
                unreg(it->fd);
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
        min_ptr = std::min(min_ptr, less_size.begin()->offset);
        
    data.erase(min_ptr - data.begin());
}

void clients_wrapper_t::write (int fd)
{
    pointers_it_t it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("write unknown fd");
    size_t wb = data.write(*it->second.socket, it->second.pointer);
    less_size.erase({it->second.pointer, it->first});
    it->second.pointer += wb;
    less_size.insert({it->second.pointer, it->first});
    if (it->second.pointer == data.end())
    {
        epoll.reg(fd, EPOLLIN);
        not_registred.insert(it->first);
    }
    shrink_to_fit();
}

std::string clients_wrapper_t::read (int fd)
{
    pointers_it_t it = pointers.find(fd);
    if (it == pointers.end())
        throw std::runtime_error("read for unknown fd");
    
    char buff [64 + sizeof(find_pattern)];
    ssize_t rb = it->second.socket->read(buff, sizeof(buff));
    if (rb < 0)
    {
        if (errno != EINTR)
            throw std::runtime_error("error read");
        else
            return std::string();
    }
    
    std::string ans = it->second.readed_data + std::string(buff, rb);
    
    size_t pos = 0;
    while (pos < ans.size())
    {
        size_t cur_inc = inc_command_ptr(ans[pos]);
        if (pos + cur_inc <= ans.size())
            pos += cur_inc;
        else
            break;
    }
    
    it->second.readed_data = ans.substr(pos);
    return ans.substr(0, pos);
}

void clients_wrapper_t::append (std::string_view value)
{
    if (value.empty())
        return;
    
    size_t diff = data.add(value);
    if (diff != 0)
    {
        std::set <client_pos> new_less_size;
        std::transform(less_size.begin(), less_size.end(), 
            std::inserter(new_less_size, new_less_size.end()),
            [diff] (client_pos v) -> client_pos 
            { return {v.offset - diff, v.fd}; });
        less_size.swap(new_less_size);
        
        std::for_each(pointers.begin(), pointers.end(), 
            [diff] (decltype(pointers)::value_type & v) -> void
            { v.second.pointer -= diff; });
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
    
    bool err = 0;
    std::for_each(not_registred.begin(), not_registred.end(),
        [this, &err] (int i) -> void
        {
            try
            {
                epoll.reg(i, EPOLLIN | EPOLLOUT);
            }
            catch (...) 
            {
                err = 1;
            }
        });
    not_registred.clear();
    
    if (err)
        throw std::runtime_error("error register in epoll");
}

