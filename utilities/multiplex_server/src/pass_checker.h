#ifndef PASS_CHECKER_H
#define PASS_CHECKER_H

#include <memory>
#include <sys/epoll.h>
#include <openssl/sha.h>
#include "check_password.h"
#include "epoll_wrapper.h"
#include "sender_data.h"

template <typename Socket>
struct pass_checker
{
    pass_checker (int _fd, epoll_wraper & _epoll);
    ~pass_checker ();
    
    pass_checker (pass_checker const &) = delete;
    pass_checker & operator = (pass_checker const &) = delete;
    pass_checker (pass_checker &&) = delete;
    pass_checker & operator = (pass_checker &&) = delete;
    
    void write ();
    void read ();
    
    bool is_ready () const noexcept
    {
        return ready;
    }
    
    bool is_acc () const noexcept
    {
        return acc;
    }
    
    int file_descriptor () const noexcept
    {
        return sock.fd();
    }

private:
    void check_pass ();
    
    Socket sock;
    epoll_wraper & epoll;
    bool ready;
    bool acc;
    size_t send_ptr;
    
    
    union
    {
        uint32_t value;
        char bytes[sizeof(value)];
    } pass_size;
    std::unique_ptr <char []> enc_pass_buf;
    size_t recv_ptr;
    
    std::unique_ptr <char []> dec_pass_buf;
    
    RSA_wrap rsa;
    sender_data_len <Socket> sender;
   
    static constexpr const char etalon_hash [] = "\074\116\177\274\334\201\232\316\374\065\235\226\213\266\134\261\061\121\044\022\265\242\113\363\277\240\067\324\171\274\120\047"; 
    static constexpr const char salt [] = "21i:3A4t&<Ahj,DyEn_,B%gOjDrX.^hx.c2@E1y*t;VubrDG\"j #Fgtv<fr"; 
};

template <typename Socket>
pass_checker <Socket> ::pass_checker (int _fd, epoll_wraper & _epoll) :
    sock(_fd),
    epoll(_epoll),
    ready(0),
    acc(0),
    send_ptr(0),
    enc_pass_buf(),
    recv_ptr(0),
    dec_pass_buf(),
    rsa(),
    sender()
{
    if (rsa.is_init())
    {
        sender.set(sock, std::string(rsa.pub_key, rsa.pub_len));
        epoll.reg(sock.fd(), EPOLLOUT);
    }
    else
    {
        throw std::runtime_error("can't init RSA");
    }
}

template <typename Socket>
pass_checker <Socket> ::~pass_checker ()
{
    try
    {
        epoll.unreg(sock.fd());
    }
    catch (...)
    {}
}

template <typename Socket>
void pass_checker <Socket> ::write ()
{
    sender.write();
    if (sender.ready())
        epoll.reg(sock.fd(), EPOLLIN);
}

template <typename Socket>
void pass_checker <Socket> ::read ()
{
    if (ready)
        return;
    
    size_t total_size = sizeof(pass_size.value);
    if (recv_ptr < total_size)
    {
        ssize_t rb = sock.read(pass_size.bytes + recv_ptr, total_size - recv_ptr);
        if (rb == -1)
        {
            if (errno != EINTR)
                throw std::runtime_error("cant' read");
            else
                rb = 0;
        }
        recv_ptr += rb;
        if (recv_ptr == total_size)
        {
            try
            {
                enc_pass_buf = std::make_unique <char []> (pass_size.value);
                dec_pass_buf = std::make_unique <char []> (pass_size.value);
            }
            catch (...)
            {
                ready = 1;
                return;
            }
            if (pass_size.value > rsa.size())
            {
                ready = 1;
                return;
            }
        }
    }
    else
    {
        size_t cur_recv_ptr = recv_ptr - sizeof(pass_size.value);
        ssize_t rb = sock.read(enc_pass_buf.get() + cur_recv_ptr, pass_size.value - cur_recv_ptr);
        if (rb == -1)
        {
            if (errno != EINTR)
                throw std::runtime_error("cant' read");
            else
                rb = 0;
        }
        recv_ptr += rb;
        if (recv_ptr == sizeof(pass_size.value) + pass_size.value)
            check_pass();
    }
}

template <typename Socket>
void pass_checker <Socket> ::check_pass ()
{
    epoll.reg(sock.fd(), 0);
    int dec_len = rsa.decrypt(enc_pass_buf.get(), pass_size.value, dec_pass_buf.get());
    if (dec_len < 0)
        goto set_ready;
    
    SHA256_CTX context;
    if (!SHA256_Init(&context))
        goto set_ready;
    if (!SHA256_Update(&context, reinterpret_cast <unsigned char const *> (dec_pass_buf.get()), dec_len))
        goto set_ready;
    if (!SHA256_Update(&context, reinterpret_cast <unsigned char const *> (salt), sizeof(salt)))
        goto set_ready;
    char calculated_hash [SHA256_DIGEST_LENGTH];
    if (!SHA256_Final(reinterpret_cast <unsigned char *> (calculated_hash), &context))
        goto set_ready;
    
    for (size_t i = 0; i != std::min(sizeof(etalon_hash), sizeof(calculated_hash)); ++i)
    {
        if (etalon_hash[i] != calculated_hash[i])
            goto set_ready;
    }
    acc = 1;
set_ready:
    ready = 1;
}
    

#endif

