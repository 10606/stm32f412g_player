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


#endif

