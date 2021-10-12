#ifndef AUTHENTIFICATOR_H
#define AUTHENTIFICATOR_H

#include <map>
#include <list>
#include <chrono>
#include <functional>
#include <unistd.h>

#include "epoll_wrapper.h"

template <typename Checker>
struct authentificator_t
{
    authentificator_t (epoll_wraper & _epoll) noexcept :
        epoll(_epoll),
        clients(),
        pointers()
    {}
    
    ~authentificator_t ()
    {
        for (map_it_t it = pointers.begin(); it != pointers.end(); ++it)
            remove(it);
    }
    
    authentificator_t (authentificator_t const &) = delete;
    authentificator_t & operator = (authentificator_t const &) = delete;
    authentificator_t (authentificator_t &&) = delete;
    authentificator_t & operator = (authentificator_t &&) = delete;

    void add (int fd) noexcept;
    void remove (int fd) noexcept;
    std::vector <int> check ();
    void read (int fd) noexcept;
    void write (int fd) noexcept;
    
    bool have (int fd) const noexcept
    {
        return pointers.find(fd) != pointers.end();
    }
    
private:
    typedef std::chrono::time_point <std::chrono::system_clock> time_point;
    struct pass_check_with_time
    {
        Checker checker;
        time_point time;
        
        template <typename Tuple_1, typename Tuple_2>
        pass_check_with_time 
        (
            Tuple_1 && args_1, 
            Tuple_2 && args_2
        ) :
            checker(std::make_from_tuple <Checker> (std::forward <Tuple_1> (args_1))),
            time   (std::make_from_tuple <time_point>   (std::forward <Tuple_2> (args_2)))
        {}
    };
    
    typedef typename std::list <pass_check_with_time> :: iterator list_it_t;
    typedef typename std::map <int, list_it_t> :: iterator map_it_t;
    
    void remove (map_it_t it) noexcept
    {
        int fd = it->second->checker.file_descriptor();
        clients.erase(it->second);
        pointers.erase(it);
        close(fd);
    }
    
    epoll_wraper & epoll;
    std::list <pass_check_with_time> clients;
    std::map <int, list_it_t> pointers;
    
    static const size_t max_clients = 100;
};

template <typename Checker>
void authentificator_t <Checker> ::add (int fd) noexcept
{
    if (fd == -1)
        return;
    if (clients.size() > max_clients)
    {
        close(fd);
        return;
    }

    time_point to = std::chrono::system_clock::now() +
                    std::chrono::minutes(1);
        
    try
    {
        clients.emplace_front(std::make_tuple(fd, std::ref(epoll)), 
                              std::make_tuple(to));
        
        try 
        {
            pointers.insert({fd, clients.begin()});
        }
        catch (...)
        {
            clients.pop_front();
            throw;
        }
    }
    catch (...)
    {
        close(fd);
    }
}

template <typename Checker>
void authentificator_t <Checker> ::remove (int fd) noexcept
{
    map_it_t it = pointers.find(fd);
    if (it == pointers.end())
        return;
    remove(it);
}

template <typename Checker>
std::vector <int> authentificator_t <Checker> ::check ()
{
    time_point now = std::chrono::system_clock::now();
    std::vector <int> ans;

    for (list_it_t it = clients.begin(); it != clients.end();)
    {
        list_it_t next = it;
        next++;
        
        int fd = it->checker.file_descriptor();
        if (it->checker.is_ready())
        {
            pointers.erase(fd);
            if (it->checker.is_acc())
            {
                clients.erase(it);
                ans.push_back(fd);
            }
            else
            {
                clients.erase(it);
                close(fd);
            }
        }
        else if (it->time < now)
        {
            pointers.erase(fd);
            clients.erase(it);
            close(fd);
        }
        
        it = next;
    }
    
    return ans;
}
    
template <typename Checker>
void authentificator_t <Checker> ::read (int fd) noexcept
{
    map_it_t it = pointers.find(fd);
    if (it == pointers.end())
        return;
    try
    {
        it->second->checker.read();
    }
    catch (...)
    {
        remove(it);
    }
}
    
template <typename Checker>
void authentificator_t <Checker> ::write (int fd) noexcept
{
    map_it_t it = pointers.find(fd);
    if (it == pointers.end())
        return;
    try
    {
        it->second->checker.write();
    }
    catch (...)
    {
        remove(it);
    }
}


#endif

