#include "com_wrapper.h"

#include <sys/epoll.h>
#include <string.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "epoll_reg.h"

com_wrapper_t::com_wrapper_t (char const * _file_name, int _epoll_fd) :
    fd(-1),
    data(),
    pos(0),
    epoll_fd(_epoll_fd)
{
    fd = open(_file_name, O_RDWR | O_NOCTTY);
    if (fd == -1)
        throw std::runtime_error("can't open file");
    
    //cdc 
    termios cdc_config;
    tcgetattr(fd, &cdc_config);
    termios new_cdc_config = cdc_config;
    cfmakeraw(&new_cdc_config);
    tcsetattr(fd, TCSANOW, &new_cdc_config);

    if (epoll_reg(epoll_fd, fd, EPOLLIN) == -1)
        throw std::runtime_error("can't reg in epoll");
}

com_wrapper_t::~com_wrapper_t ()
{
    epoll_del(epoll_fd, fd);
    close(fd);
}

void com_wrapper_t::write ()
{
    size_t wb = data.write(fd, pos);
    data.erase(wb);
    pos += wb;
    if (pos == data.end())
        epoll_reg(epoll_fd, fd, EPOLLIN);
}

void com_wrapper_t::append (std::string_view value)
{
    size_t old_end = data.end();
    size_t diff = data.add(value);
    if (pos == old_end)
        epoll_reg(epoll_fd, fd, EPOLLIN | EPOLLOUT);
    pos -= diff;
}

std::string com_wrapper_t::read ()
{
    char buff [512];
    ssize_t rb = ::read(fd, buff, sizeof(buff));
    if (rb == -1)
    {
        if ((errno != EINTR) && (errno != EPIPE))
            throw std::runtime_error("can't read");
        else
            return std::string();
    }
    return std::string(buff, rb);
}

