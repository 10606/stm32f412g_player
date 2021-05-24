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
    buffer(nullptr),
    pos(0),
    size(0),
    capacity(0),
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
    delete [] buffer;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

void com_wrapper_t::realloc (size_t needed)
{
    size_t new_size = size - pos + delta_capacity + needed;
    char * new_buffer = new char [new_size];
    if (buffer)
    {
        memmove(new_buffer, buffer + pos, size - pos);
        delete [] buffer;
    }
    buffer = new_buffer;
    capacity = new_size;
    size -= pos;
    pos = 0;
}

void com_wrapper_t::shrink_to_fit ()
{
    if (pos < delta_capacity)
        return;
    realloc(0);
}

void com_wrapper_t::write ()
{
    ssize_t wb = ::write(fd, buffer + pos, size - pos);
    if (wb < 0)
        throw std::runtime_error("can't write");
    pos += wb;
    if (pos == size)
        epoll_reg(epoll_fd, fd, EPOLLIN);
}

void com_wrapper_t::append (std::string_view value)
{
    if (size + value.size() > capacity)
        realloc(value.size());
    size_t old_size = size;
    size += value.copy(buffer + size, value.size());
    if (pos == old_size)
        epoll_reg(epoll_fd, fd, EPOLLIN | EPOLLOUT);
}

std::string com_wrapper_t::read ()
{
    char buff [512];
    ssize_t rb = ::read(fd, buff, sizeof(buff));
    if (rb == -1)
        throw std::runtime_error("can't read");
    return std::string(buff, rb);
}

