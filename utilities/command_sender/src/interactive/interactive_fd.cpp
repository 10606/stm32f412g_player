#include "interactive.h"
#include "interactive_command.h"
#include "usb_commands.h"
#include "term_display.h"
#include "epoll_wrapper.h"

#include <string_view>
#include <vector>
#include <string>
#include <fstream>
#include <deque>
#include <queue>
#include <type_traits>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <stdio.h>
#include <signal.h>
#include <iostream>

termios term_config;
volatile bool run = 1;

void sigint_handler (int signal)
{
    run = 0;
}

void set_sig_handler (int sig_num, void (* handler) (int))
{
    struct sigaction act = {handler, 0, 0, 0, 0};
    int ret = sigaction(sig_num, &act, NULL);
    if (ret)
        std::runtime_error("can't set signal handler");
}

void cl_term ()
{
    std::cout << "\033[?25h";
    std::cout << "\033[1;1H\n";
    std::cout.flush();
    tcsetattr(STDIN_FILENO, TCSANOW, &term_config);
}

struct escape_buffer
{
    escape_buffer (int _fd) :
        fd(_fd),
        epoll(),
        cmd_from_stdin(),
        info_from_stm(),
        to_write(1, 0x0e),
        state()
    {
        if (fd == -1)
            throw std::runtime_error("fd = -1");
        
        epoll.reg(STDIN_FILENO, EPOLLIN);
        epoll.reg(fd, EPOLLIN | EPOLLOUT);
    }
    
    escape_buffer (escape_buffer &&) = delete;
    escape_buffer (escape_buffer const &) = delete;
    escape_buffer & operator = (escape_buffer &&) = delete;
    escape_buffer & operator = (escape_buffer const &) = delete;
    
    void is_in_expected ()
    {
        bool has_continue = 0;
        for (unsigned char i = 1; i != int_commands.size(); ++i)
        {
            size_t j = 0;
            for (; j != std::min(cmd_from_stdin.size(), int_commands[i].size()); ++j)
            {
                if (int_commands[i][j] != cmd_from_stdin[j])
                    break;
            }
            if (j == int_commands[i].size())
            {
                for (size_t k = 0; k != j; ++k)
                    cmd_from_stdin.pop_front();
                if (int_commands[i] == "q") 
                {
                    cl_term();
                    std::exit(0);
                }
                if (to_write.empty())
                    epoll.reg(fd, EPOLLIN | EPOLLOUT);
                to_write.push_back(i);
            }
            if (j == cmd_from_stdin.size())
                has_continue = 1;
        }
        
        if (!has_continue)
            cmd_from_stdin.pop_front();
    }
    
    void put (char c)
    {
        cmd_from_stdin.push_back(c);
        is_in_expected();
    }

    void process ()
    {
        for (epoll_wraper::e_event event : epoll.wait())
        {
            if (event.fd == fd)
            {
                if (event.mask & EPOLLIN)
                {
                    char buffer[1024];
                    ssize_t ret = read(fd, buffer, sizeof(buffer));
                    if (ret == -1)
                    {
                        if (errno != EINTR)
                            throw std::runtime_error("can't read");
                        else
                            ret = 0;
                    }
                    info_from_stm.insert(info_from_stm.end(), buffer, buffer + ret);
                    extract(info_from_stm, state);
                }
                if (event.mask & EPOLLOUT)
                {
                    ssize_t ret = write(fd, to_write.c_str(), to_write.size());
                    if (ret == -1)
                    {
                        if ((errno != EINTR) && (errno != EPIPE))
                            throw std::runtime_error("can't write");
                        else
                            ret = 0;
                    }
                    if (to_write.size() == static_cast <size_t> (ret))
                    {
                        epoll.reg(fd, EPOLLIN);
                        to_write.clear();
                    }
                    else
                    {
                        to_write.erase(to_write.begin(), to_write.begin() + ret);
                    }
                }
            }
            if ((event.mask & EPOLLIN) && event.fd == STDIN_FILENO)
            {
                char ch;
                ssize_t ret = read(STDIN_FILENO, &ch, 1);
                if (ret == 1)
                    put(ch);
            }
            if ((event.mask & EPOLLHUP) || 
                (event.mask & EPOLLRDHUP) || 
                (event.mask & EPOLLERR))
            {
                throw std::runtime_error("closed");
            }
        }
    }

    int fd;
    epoll_wraper epoll;
    std::deque <char> cmd_from_stdin;
    std::deque <char> info_from_stm;
    std::string to_write;
    state_t state;
};

void interactive (int fd)
{
    set_sig_handler(SIGINT, sigint_handler);
    set_sig_handler(SIGPIPE, sigint_handler);
    
    escape_buffer escaped(fd);

    //terminal
    tcgetattr(STDIN_FILENO, &term_config);
    termios new_term_config = term_config;
    new_term_config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term_config);

    std::cout << "\033[2J";
    std::cout << "\033[1;1H\n";
    std::cout << "\033[?25l";
    std::cout.flush();
    
    try
    {
        while (run)
        {
            escaped.process();
        }
        cl_term();
    }
    catch (...)
    {
        cl_term();
        throw;
    }
}

