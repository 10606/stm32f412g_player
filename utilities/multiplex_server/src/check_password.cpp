#include "check_password.h"

#include "epoll_wrapper.h"

#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include <unistd.h>
#include <sys/epoll.h>

#include <chrono>
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <string.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>


RSA_wrap::RSA_wrap () :
    pub_key(nullptr),
    pub_len(0),
    rsa(NULL),
    succ_init(0)
{
    BIGNUM * big_number;
    big_number = BN_new();
    if (big_number == NULL)
        goto done;
    BN_set_word(big_number, RSA_F4);
    
    rsa = RSA_new();
    if (rsa == NULL)
        goto bn_free;
    if (RSA_generate_key_ex(rsa, 4096, big_number, NULL) == 0)
        goto bn_free;
    
    BIO * pub;
    pub = BIO_new(BIO_s_mem());
    if (pub == NULL)
        goto bn_free;
    if (PEM_write_bio_RSAPublicKey(pub, rsa) == 0)
        goto bio_free;
    pub_len = BIO_pending(pub);
    try
    {
        pub_key = new char [pub_len];
    }
    catch (...)
    {
        goto bio_free;
    }
    if (BIO_read(pub, pub_key, pub_len) < 0)
        goto bio_free;

    succ_init = 1;

bio_free:
    BIO_free(pub);
bn_free:
    BN_free(big_number);
done:;
}

RSA_wrap::~RSA_wrap ()
{
    delete [] pub_key;
    if (rsa != NULL)
        RSA_free(rsa);
}

int RSA_wrap::decrypt (char const * encrypted, size_t len, char * decrypted) const 
{
    if (!succ_init)
        return -1;
    return  RSA_private_decrypt
            (
                len, 
                reinterpret_cast <unsigned char const*> (encrypted), 
                reinterpret_cast <unsigned char *> (decrypted),
                rsa, 
                RSA_PKCS1_OAEP_PADDING
            );
}


pass_checker::pass_checker (int _fd, epoll_wraper & _epoll) :
    fd(_fd),
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
        sender.set(fd, std::string(rsa.pub_key, rsa.pub_len));
        epoll.reg(fd, EPOLLOUT);
    }
    else
    {
        throw std::runtime_error("can't init RSA");
    }
}

pass_checker::~pass_checker ()
{
    try
    {
        epoll.unreg(fd);
    }
    catch (...)
    {}
}

void pass_checker::write ()
{
    sender.write();
    if (sender.ready())
        epoll.reg(fd, EPOLLIN);
}

void pass_checker::read ()
{
    if (ready)
        return;
    
    size_t total_size = sizeof(pass_size.value);
    if (recv_ptr < total_size)
    {
        ssize_t rb = ::read(fd, pass_size.bytes + recv_ptr, total_size - recv_ptr);
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
        ssize_t rb = ::read(fd, enc_pass_buf.get() + cur_recv_ptr, pass_size.value - cur_recv_ptr);
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

void pass_checker::check_pass ()
{
    epoll.reg(fd, 0);
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
    

void authentificator_t::add (int fd)
{
    if (fd == -1)
        return;
    if (clients.size() > max_clients)
        return;

    time_point to = std::chrono::system_clock::now() +
                    std::chrono::minutes(1);
        
    try
    {
        clients.emplace_front(std::make_tuple(fd, std::ref(epoll)), 
                              std::make_tuple(to));
        
        try 
        {
            pointers.insert({fd, clients.begin()});
        }
        catch (...)
        {
            clients.pop_front();
            throw;
        }
    }
    catch (...)
    {
        close(fd);
    }
}

void authentificator_t::remove (int fd)
{
    std::map <int, it_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        return;
    remove(it);
}

std::vector <int> authentificator_t::check ()
{
    time_point now = std::chrono::system_clock::now();
    std::vector <int> ans;

    for (it_t it = clients.begin(); it != clients.end();)
    {
        it_t next = it;
        next++;
        
        int fd = it->checker.file_descriptor();
        if (it->checker.is_ready())
        {
            pointers.erase(fd);
            if (it->checker.is_acc())
            {
                clients.erase(it);
                ans.push_back(fd);
            }
            else
            {
                clients.erase(it);
                close(fd);
            }
        }
        else if (it->time < now)
        {
            pointers.erase(fd);
            clients.erase(it);
            close(fd);
        }
        
        it = next;
    }
    
    return ans;
}
    
void authentificator_t::read (int fd)
{
    std::map <int, it_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        return;
    try
    {
        it->second->checker.read();
    }
    catch (...)
    {
        remove(it);
    }
}
    
void authentificator_t::write (int fd)
{
    std::map <int, it_t> :: iterator it = pointers.find(fd);
    if (it == pointers.end())
        return;
    
    try
    {
        it->second->checker.write();
    }
    catch (...)
    {
        remove(it);
    }
}

