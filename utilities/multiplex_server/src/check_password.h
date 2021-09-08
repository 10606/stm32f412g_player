#ifndef CHECK_PASSWORD_H
#define CHECK_PASSWORD_H

#include <openssl/rsa.h>
#include <unistd.h>

#include <stdint.h>
#include <stddef.h>
#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <list>

#include "epoll_wrapper.h"
#include "sender_data.h"

struct RSA_wrap
{
    RSA_wrap ();
    ~RSA_wrap ();

    RSA_wrap (RSA_wrap const &) = delete;
    RSA_wrap & operator = (RSA_wrap const &) = delete;
    RSA_wrap (RSA_wrap &&) = delete;
    RSA_wrap & operator = (RSA_wrap &&) = delete;
    
    bool is_init () const noexcept
    {
        return succ_init;
    }
    
    size_t size () const 
    {
        if (!succ_init)
            return 0;
        return RSA_size(rsa);
    }
    
    int decrypt (char const * encrypted, size_t len, char * decrypted) const;
    
    char * pub_key;
    size_t pub_len;
    
private:
    RSA * rsa;
    bool succ_init;
};


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
    
    bool is_acc () const
    {
        return acc;
    }
    
    int file_descriptor () const noexcept
    {
        return fd;
    }

private:
    void check_pass ();
    
    int fd;
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
    sender_data_len sender;
   
    static constexpr const char etalon_hash [] = "\074\116\177\274\334\201\232\316\374\065\235\226\213\266\134\261\061\121\044\022\265\242\113\363\277\240\067\324\171\274\120\047"; 
    static constexpr const char salt [] = "21i:3A4t&<Ahj,DyEn_,B%gOjDrX.^hx.c2@E1y*t;VubrDG\"j #Fgtv<fr"; 
};


struct authentificator_t
{
    authentificator_t (epoll_wraper & _epoll) :
        epoll(_epoll),
        clients(),
        pointers()
    {}
    
    ~authentificator_t ()
    {
        for (std::map <int, it_t> :: iterator it = pointers.begin(); it != pointers.end(); ++it)
        {
            try
            {
                remove (it);
            }
            catch (...)
            {}
        }
    }

    void add (int fd);
    void remove (int fd);
    std::vector <int> check ();
    void read (int fd);
    void write (int fd);
    
    bool have (int fd)
    {
        return pointers.find(fd) != pointers.end();
    }
    
private:
    typedef std::chrono::time_point <std::chrono::system_clock> time_point;
    typedef std::list <std::pair <pass_checker, time_point> > :: iterator it_t;
    
    void remove (std::map <int, it_t> :: iterator it)
    {
        int fd = it->second->first.file_descriptor();
        clients.erase(it->second);
        pointers.erase(it);
        epoll.unreg(fd);
        close(fd);
    }
    
    epoll_wraper & epoll;
    std::list <std::pair <pass_checker, time_point> > clients;
    std::map <int, it_t> pointers;
    
    static const size_t max_clients = 100;
};


#endif

