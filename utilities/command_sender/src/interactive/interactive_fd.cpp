#include "interactive.h"
#include "interactive_command.h"
#include "usb_commands.h"
#include "display.h"

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
bool run = 1;

void sigint_handler (int signal)
{
    if (signal != SIGINT)
        return;
    run = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term_config);
    std::cout << "\033[?25h";
    std::cout.flush();
}


struct escape_buffer
{
    escape_buffer (int _fd) :
        fd(_fd),
        epoll(epoll_create(3)),
        cur(),
        to_write(1, 0x0e),
        state(4)
    {
        if (fd == -1)
            throw std::runtime_error("fd = -1");
        if (epoll == -1)
            throw std::runtime_error("cant't create epoll");
        
        epoll_data_t stdin_data;
        stdin_data.fd = STDIN_FILENO;
        epoll_event stdin_event{EPOLLIN | EPOLLERR | EPOLLHUP, stdin_data};
        if (epoll_ctl(epoll, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event))
            throw std::runtime_error("can't add stdin to epoll");

        epoll_data_t fd_data;
        fd_data.fd = fd;
        epoll_event fd_event{EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP, fd_data};
        if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &fd_event))
            throw std::runtime_error("can't add fd to epoll");
    }
    
    void mod_fd (uint32_t flag)
    {
        epoll_data_t fd_data;
        fd_data.fd = fd;
        epoll_event fd_event{flag | EPOLLERR | EPOLLHUP, fd_data};
        if (epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &fd_event))
            throw std::runtime_error("can't mod fd in epoll");
    }

    void is_in_expected ()
    {
        //std::cout << "cur.size " << cur.size() << "\n";
        bool has_continue = 0;
        for (unsigned char i = 1; i != std::extent <decltype(int_commands)>::value; ++i)
        {
            size_t j = 0;
            for (; j != std::min(cur.size(), std::strlen(int_commands[i])); ++j)
            {
                if (int_commands[i][j] != cur[j])
                {
                    break;
                }
            }
            if (j == std::strlen(int_commands[i]))
            {
                for (size_t k = 0; k != j; ++k)
                {
                    cur.pop_front();
                }
                //std::cout << "i " << static_cast <int> (i) << "\n";
                if (i == 0x0f)
                {
                    tcsetattr(STDIN_FILENO, TCSANOW, &term_config);
                    std::cout << "\033[?25h";
                    std::cout.flush();
                    std::exit(0);
                }
                //std::cerr << "send " << static_cast <uint32_t> (i) << "\n";
                if (to_write.empty())
                    mod_fd(EPOLLIN | EPOLLOUT);
                to_write.push_back(i);
            }
            if (j == cur.size())
            {
                has_continue = 1;
            }
        }
        
        if (!has_continue)
        {
            cur.pop_front();
        }
    }
    
    void put (char c)
    {
        cur.push_back(c);
        is_in_expected();
    }

    void process ()
    {
        //std::cout << ">> wait in epoll\n";
        epoll_event event[2];
        int cnt = epoll_wait(epoll, event, sizeof(event), -1); 
        if (cnt == -1)
            throw std::runtime_error("can't epoll wait");
        //std::cout << "<< wait in epoll\n";
        for (int i = 0; i != cnt; ++i)
        {
            if ((event[i].events & EPOLLIN) && event[i].data.fd == fd)
            {
                //std::cout << "== fd in\n";
                char buffer[1024];
                ssize_t ret = read(fd, buffer, sizeof(buffer));
                if (ret == -1)
                    throw std::runtime_error("can't read");
                readed += std::string(buffer, ret);
                extract(readed, state);
            }
            if ((event[i].events & EPOLLOUT) && event[i].data.fd == fd)
            {
                //std::cout << "== fd out\n";
                ssize_t ret = write(fd, to_write.c_str(), to_write.size());
                if (ret == -1)
                    throw std::runtime_error("can't write");
                if (to_write.size() == static_cast <size_t> (ret))
                {
                    mod_fd(EPOLLIN);
                    to_write.clear();
                }
                else
                    to_write = to_write.substr(ret);
            }
            if ((event[i].events & EPOLLIN) && event[i].data.fd == STDIN_FILENO)
            {
                //std::cout << "== stdin\n";
                char ch;
                read(STDIN_FILENO, &ch, 1);
                //char ch = getchar();
                //std::cout << "read " <<  std::hex << static_cast <int> (ch) << "\n";
                put(ch);
            }
        }
    }
    
    int fd;
    int epoll;
    std::deque <char> cur;
    std::string to_write;
    std::string readed;
    size_t state;
};

void interactive (std::string const & path)
{
    int fd = open(path.c_str(), O_RDWR | O_NOCTTY);
    escape_buffer escaped(fd);

    //terminal
    //termios term_config;
    tcgetattr(STDIN_FILENO, &term_config);
    termios new_term_config = term_config;
    new_term_config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term_config);

    signal(SIGINT, sigint_handler);

    //cdc 
    termios cdc_config;
    tcgetattr(fd, &cdc_config);
    termios new_cdc_config = cdc_config;
    cfmakeraw(&new_cdc_config);
    //new_cdc_config.c_lflag |= (NOFLSH | IGNBRK);
    //new_cdc_config.c_lflag &= ~(ICANON | ECHO);
    //new_cdc_config.c_cc[VMIN] = 1;
    //new_cdc_config.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSANOW, &new_cdc_config);

    std::cout << "\033[3J";
    std::cout << "\033\143";
    std::cout << "\033[?25l";
    std::cout.flush();
    
    try
    {
        while (run)
        {
            escaped.process();
        }
    }
    catch (...)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &term_config);
        std::cout << "\033[?25h";
        std::cout.flush();
    }
}
