#include "com_wrapper.h"

#include <sys/epoll.h>
#include <string.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "epoll_wrapper.h"

com_wrapper_t::com_wrapper_t (char const * _file_name, epoll_wraper & _epoll) :
    fd(-1),
    data(16),
    pos(0),
    epoll(_epoll)
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

    epoll.reg(fd, EPOLLIN);
}

com_wrapper_t::~com_wrapper_t ()
{
    epoll.unreg(fd);
    close(fd);
}

void com_wrapper_t::write ()
{
    size_t wb = data.write(fd, pos);
    data.erase(wb);
    pos += wb;
    if (pos == data.end())
        epoll.reg(fd, EPOLLIN);
}

void com_wrapper_t::append (std::string_view value)
{
    if (value.empty())
        return;

    size_t old_end = data.end();
    size_t diff = data.add(value);
    if (pos == old_end)
        epoll.reg(fd, EPOLLIN | EPOLLOUT);
    pos -= diff;
}

std::string com_wrapper_t::read ()
{
    char buff [512];
    ssize_t rb = ::read(fd, buff, sizeof(buff));
    if (rb == -1)
    {
        if (errno != EINTR)
            throw std::runtime_error("can't read");
        else
            return std::string();
    }
    return std::string(buff, rb);
}

