#include "web_socket.h"

#include <iostream>
#include <assert.h>


ssize_t web_socket_reader_t::read (char * buffer, size_t size)
{
    if (status != status_t::data)
    {
        ssize_t ret = ::read(fd, data.value + cur_pos, cur_size - cur_pos);
        if (ret == -1)
            return ret;
                    
        cur_pos += ret;
        if (cur_pos == cur_size)
        {
            cur_pos = 0;
            switch (status)
            {
            case status_t::header:
                fin = data.header.fin;
                has_mask = data.header.mask;
                opcode = data.header.opcode;
                
                if (data.header.len < 126)
                {
                    data_len = reinterpret_cast <uint8_t> (data.header.len);
                    status = status_t::mask;
                    cur_size = sizeof(data.mask);
                }
                else if (data.header.len == 126)
                {
                    status = status_t::len_2;
                    cur_size = sizeof(data.len_2);
                }
                else if (data.header.len == 127)
                {
                    status = status_t::len_8;
                    cur_size = sizeof(data.len_8);
                }
                break;
                
            case status_t::len_2:
                data_len = data.len_2;
                status = status_t::mask;
                cur_size = sizeof(data.mask);
                break;
                
            case status_t::len_8:
                data_len = data.len_8;
                status = status_t::mask;
                cur_size = sizeof(data.mask);
                break;
                
            case status_t::mask:
                memcpy(mask, data.mask, sizeof(mask));
                status = status_t::data;
                cur_size = data_len;   
                break;
                
            case status_t::data:
                /*impossible*/;
            }
        }
        
        if ((status == status_t::mask) && !has_mask)
        {
            status = status_t::data;
            cur_size = data_len;   
        }
        return 0;
    }
    else
    {
        ssize_t ret = ::read(fd, buffer, std::min(cur_size - cur_pos, size));
        if (ret == -1)
            return ret;
        
        cur_pos += ret;
        if (cur_pos == cur_size)
        {
            status = status_t::header;
            cur_pos = 0;
            cur_size = sizeof(web_socket_header_t);
        }
        
        if (has_mask)
        {
            for (ssize_t i = 0; i != ret; ++i)
                buffer[i] ^= mask[i % 4];
        }
        return ret;
    }
}


ssize_t web_socket_writer_t::write (char const * data, size_t size) 
{
    ssize_t ret;
    switch (status)
    {
    case status_t::wait:
        web_socket_header_t base_header;
        
        if (size < 126)
        {
            base_header = {0x02, 0, 0, 0, 1, static_cast <uint8_t> (size), 0};
            cur_header_size = 0;
        }
        else 
        {
            if (size < (1lu << 16))
            {
                base_header = {0x02, 0, 0, 0, 1, 126, 0};
                uint16_t len = size;
                memcpy(header + sizeof(web_socket_header_t), &len, sizeof(len));
                cur_header_size = sizeof(len);
            }
            else
            {
                base_header = {0x02, 0, 0, 0, 1, 127, 0};
                uint64_t len = size;
                memcpy(header + sizeof(web_socket_header_t), &len, sizeof(len));
                cur_header_size = sizeof(len);
            }
        }
        memcpy(header, &base_header, sizeof(base_header));
        cur_pos = 0;
        cur_header_size += sizeof(base_header);
        cur_data_size = size;
        status = status_t::header;
        // without break!
        
    case status_t::header:
        ret = ::write(fd, header + cur_pos, cur_header_size - cur_pos);
        if (ret == -1)
            return ret;
        
        cur_pos += ret;
        if (cur_pos == cur_header_size)
            status = status_t::data;
        return 0;
        
    case status_t::data:
        ret = ::write(fd, data, std::min(size, cur_data_size));
        cur_data_size -= ret;
        if (cur_data_size == 0)
            status = status_t::wait;
        return ret;
    }

    return 0;
}

namespace
{

size_t base64_encode (char * dst, unsigned char * src, size_t count)
{
    static const std::string_view base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t count_ceil = count - count % 3;
    size_t pos = 0;

    for (size_t i = 0; i < count_ceil; i += 3)
    {
        dst[pos++] = base64_chars[ (src[i]   & 0xfc) >> 2];
        dst[pos++] = base64_chars[((src[i]   & 0x03) << 4) + ((src[i+1] & 0xf0) >> 4)];
        dst[pos++] = base64_chars[((src[i+1] & 0x0f) << 2) + ((src[i+2] & 0xc0) >> 6)];
        dst[pos++] = base64_chars[  src[i+2] & 0x3f ];
    }
    
    if (count_ceil != count)
    {
        dst[pos++] = base64_chars[ (src[count_ceil] & 0xfc) >> 2];
        dst[pos++] = base64_chars[((src[count_ceil] & 0x03) << 4) + 
                                  (count_ceil + 1 < count ? (src[count_ceil+1] & 0xf0) >> 4 : 0)];
        dst[pos++] = (count_ceil + 1 < count ? base64_chars[((src[count_ceil+1] & 0x0f) << 2)] : '=');
        dst[pos++] = '=';
    }
    return pos;
}

}


template <typename T, size_t N>
std::ostream & operator << (std::ostream & s, std::array <T, N> const & data)
{
    for (T const & v : data)
        s << v;
    return s;
}

void web_socket_init_t::read ()
{
    char buffer[1024];
    ssize_t ret = ::read(fd, buffer, sizeof(buffer));
    if (ret == -1)
    {
        if (errno != EINTR)
            throw std::runtime_error("error read at web socket initialization");
        else
            ret = 0;
    }
    
    size_t i = 0;
    if (!hash_ready)
    {
        for (i = 0; i != (static_cast <size_t> (ret)) && (pos != value.size()); ++i)
        {
            if (value[pos] != buffer[i])
                [[likely]]
                pos = 0;
            
            pos += (value[pos] == buffer[i]);
        }
        
        for (; i != static_cast <size_t> (ret); ++i)
        {
            if (buffer[i] == '\r')
            {
                key += magic;
                SHA_CTX ctx;
                std::array <unsigned char, SHA_DIGEST_LENGTH> raw_hash;
                SHA1_Init(&ctx);
                SHA1_Update(&ctx, key.c_str(), key.size());
                SHA1_Final(raw_hash.data(), &ctx);
                std::array <char, 64> base64_hash;
                size_t hash_size = base64_encode(base64_hash.data(), raw_hash.data(), raw_hash.size());
                hash = head + std::string(base64_hash.data(), hash_size) + tail;
                pos = 0;
                hash_ready = 1;
                break;
            }
            key += buffer[i];
            
            if (key.size() > 1000)
            {
                key.clear();
                hash_ready = 1;
                header_readed = 1;
                pos = hash.size();
                epoll.reg(fd, 0);
                return;
            }
        }
    }
    if (hash_ready)
    {
        std::string_view end_pattern = "\r\n\r\n"; 

        for (; i != (static_cast <size_t> (ret)) && (pos != end_pattern.size()); ++i)
        {
            if (end_pattern[pos] != buffer[i])
                [[likely]]
                pos = 0;
            
            pos += (end_pattern[pos] == buffer[i]);
        }
        if (pos == end_pattern.size())
        {
            header_readed = 1;
            pos = 0;
            epoll.reg(fd, EPOLLOUT);
        }
    }
}

void web_socket_init_t::write ()
{
    if (!hash_ready)
        return;
    
    ssize_t ret = ::write(fd, hash.data() + pos, hash.size() - pos);
    if (ret == -1)
    {
        if ((errno != EINTR) && (errno != EPIPE))
            throw std::runtime_error("error write at web socket initialization");
        else
            ret = 0;
    }
    
    pos += ret;
    if (pos == hash.size())
    {
        accept = 1;
        epoll.reg(fd, 0);
    }
}

std::string_view web_socket_init_t::value = "\r\nSec-WebSocket-Key: ";
std::string_view web_socket_init_t::magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string web_socket_init_t::head = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
std::string web_socket_init_t::tail = "\r\n\r\n";

