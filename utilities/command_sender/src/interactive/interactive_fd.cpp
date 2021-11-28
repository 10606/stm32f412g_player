#include "interactive.h"
#include "interactive_command.h"
#include "usb_commands.h"
#include "term_display.h"
#include "epoll_wrapper.h"
#include "playlist_structures.h"
#include "convert_custom.h"

#include <string_view>
#include <vector>
#include <string>
#include <fstream>
#include <map>
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
    
    void search_input ()
    {
        for (size_t i = 0; i != cmd_from_stdin.size(); ++i)
        {
            size_t index = finder.need_continue - 1;
            if ((cmd_from_stdin[i] == 0x7f)  || // linux / xfce
                (cmd_from_stdin[i] == 0x08))    // xterm
            {
                if (!finder.pattern[index].empty())
                    finder.pattern[index].pop_back();
                else if (finder.need_continue == 2)
                    finder.need_continue--;
            }
            else if (cmd_from_stdin[i] == '\n')
            {
                finder.need_continue++;
                if (finder.need_continue == 3)
                {
                    std::basic_string <uint8_t> group_name = utf8_to_custom(finder.pattern[0]);
                    std::basic_string <uint8_t> song_name = utf8_to_custom(finder.pattern[1]);
                
                    find_pattern pattern;
                    pattern.group_len = group_name.size();
                    pattern.song_len  = song_name.size();
                    memcpy(pattern.group_name, group_name.c_str(), std::min(group_name.size(), sizeof(find_pattern::group_name)));
                    memcpy(pattern.song_name,  song_name.c_str(),  std::min(song_name.size(),  sizeof(find_pattern::song_name)));
                    
                    char pattern_bytes[1 + sizeof(pattern)];
                    pattern_bytes[0] = static_cast <char> (128);
                    memcpy(pattern_bytes + 1, &pattern, sizeof(pattern));
                    
                    if (to_write.empty())
                        epoll.reg(fd, EPOLLIN | EPOLLOUT);
                    to_write += std::string(pattern_bytes, sizeof(pattern_bytes));

                    finder.pattern[0].clear();
                    finder.pattern[1].clear();
                    finder.need_continue = 0;
                    cmd_from_stdin.erase(cmd_from_stdin.begin(), cmd_from_stdin.begin() + i + 1);
                    
                    display_search(finder.pattern);
                    return;
                }
            }
            else
            {
                static const std::array <size_t, 2> max_sizes = {3 * sizeof(find_pattern::group_name), 
                                                                 3 * sizeof(find_pattern::song_name)};
                if (max_sizes[index] > finder.pattern[index].size())
                    finder.pattern[index].push_back(cmd_from_stdin[i]);
            }
        }

        display_search(finder.pattern);
        cmd_from_stdin.clear();
    }

    bool is_in_expected ()
    {
        if (finder.need_continue)
        {
            search_input();
            return !finder.need_continue;
        }
        
        
        std::deque <char> ::iterator it = cmd_from_stdin.begin();
        for (; it != cmd_from_stdin.end(); ++it)
        {
            if ((*it < '0') || (*it > '9'))
                break;
            number = (number * 10 + (*it - '0')) % 1000000000;
        }
        
        bool has_continue = 0;
        for (unsigned char i = 1; i != int_commands.size(); ++i)
        {
            std::pair <std::deque <char> ::iterator, std::string_view::iterator> pos = 
                std::mismatch(it, cmd_from_stdin.end(),
                              int_commands[i].begin(), int_commands[i].end());
            if (pos.second == int_commands[i].end())
            {
                it = pos.first;
                
                if (int_commands[i] == quit) 
                {
                    cl_term();
                    std::exit(0);
                }
                else if (i >= 0x12) // command with args
                {
                    if (i == 0x12)
                    {
                        finder.need_continue = 1;
                        number = 0;
                        cmd_from_stdin.erase(cmd_from_stdin.begin(), it);
                        search_input();
                        return !finder.need_continue;
                    }
                    else if (i == 0x13)
                    {
                        char cmd_bytes[1 + sizeof(position_t)];
                        cmd_bytes[0] = static_cast <char> (128 + i - 0x12);
                        memcpy(cmd_bytes + 1, &number, sizeof(position_t));

                        if (to_write.empty())
                            epoll.reg(fd, EPOLLIN | EPOLLOUT);
                        to_write += std::string(cmd_bytes, sizeof(cmd_bytes));

                        number = 0;
                        continue;
                    }
                }
                
                if (to_write.empty())
                    epoll.reg(fd, EPOLLIN | EPOLLOUT);
                to_write.push_back(i);
                number = 0;
            }
            if (pos.first == cmd_from_stdin.end())
                has_continue = 1;
        }
        
        cmd_from_stdin.erase(cmd_from_stdin.begin(), it);
        if (!has_continue && !cmd_from_stdin.empty())
        {
            cmd_from_stdin.pop_front();
            number = 0;
            display_number(number);
            return 1;
        }
        display_number(number);
        return 0;
    }
    
    void put (char c)
    {
        cmd_from_stdin.push_back(c);
        while (is_in_expected());
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

    struct long_command
    {
        uint8_t need_continue = 0;
        std::array <std::string, 2> pattern;
    };
    
    int fd;
    long_command finder;
    position_t number = 0;
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

