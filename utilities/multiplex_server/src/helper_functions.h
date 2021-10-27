#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <sys/epoll.h>
#include "epoll_wrapper.h"
#include "tcp_server_sock.h"
#include "authentificator.h"


template <typename Init>
bool just_do (authentificator_t <Init> & init, epoll_wraper::e_event event) noexcept
{
    if (init.have(event.fd))
    {
        try
        {
            if (event.mask & EPOLLIN)
                init.read(event.fd);
            if (event.mask & EPOLLOUT)
                init.write(event.fd);
        }
        catch (...)
        {
            init.remove(event.fd);
        }
        return 1;
    }
    return 0;
}

template <typename Init>
bool accept_for_auth (tcp_server_sock_t & socket, authentificator_t <Init> & init, epoll_wraper::e_event event) noexcept
{
    if (event.fd == socket.file_descriptor())
    {
        if (!(event.mask & EPOLLIN))
            return 1;
        int fd;
        try
        {
            fd = socket.accept();
        }
        catch (...)
        {}
        init.add(fd);
        return 1;
    }
    return 0;
}

template <typename Init>
void check_all (authentificator_t <Init> & auth, epoll_wraper & epoll_wrap, std::function <void (int)> fn_add) noexcept
{
    try
    {
        std::vector <int> need_add = auth.check();
        std::for_each(need_add.begin(), need_add.end(), 
            [fn_add, &epoll_wrap] (int fd) -> void 
            {
                try
                {
                    fn_add(fd);
                }
                catch (...)
                {
                    epoll_wrap.unreg(fd);
                    close(fd);
                }
            });
    }
    catch (...)
    {}
}

template <typename Socket>
bool accept_and_close (Socket & socket, epoll_wraper::e_event event) noexcept
{
    if ((event.fd == socket.file_descriptor()) &&
        (event.mask & EPOLLIN))
    {
        int fd = -1;
        try
        {
            fd = socket.accept();
        }
        catch (...)
        {}
        if (fd != -1)
            close(fd);
        return 1;
    }
    return 0;
}


#endif

