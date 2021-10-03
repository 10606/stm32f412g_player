#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <stdint.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <unistd.h>

#include "epoll_wrapper.h"

struct web_socket_header_t
{
    //uint8_t fin:1, rsv1:1, rsv2:1, rsv3:1, opcode:4;
    //uint8_t mask:1, len:7;
    uint8_t opcode:4, rsv3:1, rsv2:1, rsv1:1, fin:1;  
    uint8_t len:7, mask:1;
};

struct web_socket_reader_t
{
    web_socket_reader_t (int _fd) :
        fd(_fd),
        status(status_t::header),
        cur_pos(0),
        cur_size(sizeof(web_socket_header_t))
    {}
        
    ssize_t read (char * buffer, size_t size);

    enum class status_t
    {
        header,
        len_2,
        len_8,
        mask, //just state maybe don't need read mask
        data
    };

    union data_t
    {
        web_socket_header_t header;
        uint16_t len_2;
        uint64_t len_8;
        uint8_t mask[4];
        
        char value[std::max(sizeof(header), sizeof(len_8))];
        
        static_assert(sizeof(uint64_t) == sizeof(size_t));
    };

    int fd;
    
    status_t status;
    data_t data;
    size_t cur_pos;
    size_t cur_size;
    bool fin;
    bool has_mask;
    uint8_t mask[4];
    uint8_t opcode;
    size_t data_len;
};



struct web_socket_writer_t
{
    web_socket_writer_t (int _fd) :
        fd(_fd),
        status(status_t::wait)
    {}
        
    ssize_t write (char const * data, size_t size);
    
    enum class status_t
    {
        wait,
        header,
        data
    };
    
    int fd;
    status_t status;
    size_t cur_data_size;
    char header[sizeof(web_socket_header_t) + sizeof(uint64_t)];
    size_t cur_header_size;
    size_t cur_pos;
};

struct web_socket_init_t
{
    web_socket_init_t (int _fd, epoll_wraper & _epoll) :
        fd(_fd),
        epoll(_epoll),
        pos(0),
        key(),
        hash_ready(0),
        header_readed(0),
        accept(0)
    {
        epoll.reg(fd, EPOLLIN);
    }

    ~web_socket_init_t ()
    {
        try
        {
            epoll.unreg(fd);
        }
        catch (...)
        {}
    }
    
    void read ();
    void write ();
    
    bool is_ready () const noexcept
    {
        return hash_ready && (pos == hash.size());
    }
    
    bool is_acc () const noexcept
    {
        return accept;
    }
    
    int file_descriptor () const noexcept
    {
        return fd;
    }

    static std::string_view value; // all suff link go to 0
    static std::string_view magic;

    static std::string head;
    static std::string tail;
    
    int fd;
    epoll_wraper & epoll;
    size_t pos;
    std::string key;
    bool hash_ready;
    bool header_readed;
    bool accept;
    std::string hash;
};


struct web_socket_t
{
    web_socket_t (int _fd) :
        fd_v(_fd),
        reader(_fd),
        writer(_fd)
    {}

    ssize_t read (char * buffer, size_t size)
    {
        return reader.read(buffer, size);
    }
    
    ssize_t write (char const * data, size_t size)
    {
        return writer.write(data, size);
    }

    int fd () const noexcept
    {
        return fd_v;
    }

private:
    int fd_v;
    web_socket_reader_t reader;
    web_socket_writer_t writer;
};


#endif

