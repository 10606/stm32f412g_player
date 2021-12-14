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
    
    template <typename T>
    void add_packet (uint8_t header, T value)
    {
        char cmd_bytes[sizeof(header) + sizeof(value)];
        memcpy(cmd_bytes, &header, sizeof(header));
        memcpy(cmd_bytes + sizeof(header), &value, sizeof(value));

        if (to_write.empty())
            epoll.reg(fd, EPOLLIN | EPOLLOUT);
        to_write += std::string(cmd_bytes, sizeof(cmd_bytes));
    }
    
    bool check_escape_sequence (uint8_t cmd)
    {
        if (cmd == 0x1b) // esc
        {
            finder.status = long_command::escape;
            return 1;
        }
        if (cmd == 0x9b) // csi
        {
            finder.status = long_command::open_sb;
            return 1;
        }
        if (finder.status == long_command::escape)
        {
            if (cmd == '[')
                finder.status = long_command::open_sb;
            else 
                finder.status = long_command::none;
            return 1;
        }
        if ((finder.status == long_command::open_sb) &&
            (cmd >= 64) &&
            (cmd <= 126))
        {
            finder.status = long_command::none;
            return 1;
        }

        return finder.status != long_command::none;
    }
    
    void search_input ()
    {
        for (size_t i = 0; i != cmd_from_stdin.size(); ++i)
        {
            if (check_escape_sequence(cmd_from_stdin[i]))
                continue;
            
            size_t index = finder.need_continue - 1;
            if ((cmd_from_stdin[i] == 0x7f)  || // linux / xfce
                (cmd_from_stdin[i] == 0x08))    // xterm
            {
                if (!finder.pattern[index].empty())
                {
                    while (!finder.pattern[index].empty() &&
                            static_cast <uint8_t> (finder.pattern[index].back()) >> 6 == 0b10)
                        finder.pattern[index].pop_back();
                    if (!finder.pattern[index].empty())
                        finder.pattern[index].pop_back();
                }
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
                    pattern.group_len = std::min(group_name.size(), sizeof(find_pattern::group_name));
                    pattern.song_len  = std::min(song_name.size(),  sizeof(find_pattern::song_name));
                    memcpy(pattern.group_name, group_name.c_str(), pattern.group_len);
                    memcpy(pattern.song_name,  song_name.c_str(),  pattern.song_len);
                    
                    add_packet <find_pattern> (128, pattern);

                    finder.pattern[0].clear();
                    finder.pattern[1].clear();
                    finder.need_continue = 0;
                    std::cout << "\033[?25l";
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
            static const position_t mod = 100000000;
            number %= mod;
            number = number * 10 + (*it - '0');
        }
        
        bool has_continue = 0;
        for (unsigned char i = 1; i != int_commands.size(); ++i)
        {
            if (it == cmd_from_stdin.end())
                break;
            
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
                else if (i >= long_commands_separator) // command with args
                {
                    switch (i - long_commands_separator)
                    {
                    case 0:
                        finder.need_continue = 1;
                        number = 0;
                        cmd_from_stdin.erase(cmd_from_stdin.begin(), it);
                        std::cout << "\033[?25h";
                        search_input();
                        return !finder.need_continue;
                    case 1:
                        add_packet <position_t> (128 + i - long_commands_separator, number);
                        number = 0;
                        break;
                    }
                }
                else
                {
                    if (to_write.empty())
                        epoll.reg(fd, EPOLLIN | EPOLLOUT);
                    to_write.push_back(i);
                    number = 0;
                    --i;
                }
            }
            if (pos.first == cmd_from_stdin.end())
            {
                has_continue = 1;
                break;
            }
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
    
    void put (std::string_view c)
    {
        cmd_from_stdin.insert(cmd_from_stdin.end(), c.begin(), c.end());
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
                char buffer[10];
                ssize_t ret = read(STDIN_FILENO, &buffer, sizeof(buffer));
                if (ret > 0)
                    put(std::string_view (buffer, ret));
            }
            if ((event.mask & EPOLLHUP) || 
                (event.mask & EPOLLRDHUP) || 
                (event.mask & EPOLLERR))
            {
                throw std::runtime_error("closed");
            }
            
            if (finder.need_continue)
            {
                set_cursor_pos_search(finder.pattern, finder.need_continue - 1);
            }
        }
    }

    struct long_command
    {
        enum escape_status
        {
            none,
            escape,
            open_sb,
        };
        
        escape_status status = none;
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

