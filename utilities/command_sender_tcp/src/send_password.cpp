#include "epoll_reg.h"
#include "epoll_wrapper.h"
#include "recver_data.h"
#include "sender_data.h"
#include "interactive.h"

#include <openssl/rsa.h> 
#include <openssl/pem.h> 
#include <openssl/bio.h> 

#include <stdexcept>
#include <optional>
#include <string_view>
#include <string>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <iostream>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

std::string enc_pass (std::string_view password, std::string_view pub_key)
{
    std::string ans;
    bool has_err = 1;
    char * encrypt;

    BIO * pub;
    RSA * rsa = NULL;
    
    pub = BIO_new(BIO_s_mem());
    if (pub == NULL)
    {
        std::cerr << "can't create BIO\n";
        goto done;
    }
    if (BIO_write(pub, pub_key.data(), pub_key.size()) < 0)
    {
        std::cerr << "can't write to BIO\n";
        goto bio_free;
    }
    if (PEM_read_bio_RSAPublicKey(pub, &rsa, NULL, NULL) == NULL)
    {
        std::cerr << "can't PEM read BIO\n";
        goto bio_free;
    }

    try
    {
        encrypt = new char [RSA_size(rsa)];
    }
    catch (...)
    {
        goto rsa_free;
    }
    
    int encrypt_len;
    if ((encrypt_len =  RSA_public_encrypt
                        (
                            password.size(), 
                            reinterpret_cast <unsigned char const *> (password.data()), 
                            reinterpret_cast <unsigned char *> (encrypt),
                            rsa, 
                            RSA_PKCS1_OAEP_PADDING
                        )) == -1) 
    {
        std::cerr << "can't encrypt\n";
        goto encrypt_free;
    }
    
    has_err = 0;
    ans = std::string(encrypt, encrypt_len);
encrypt_free:
    delete [] encrypt;
rsa_free:
    RSA_free(rsa);
bio_free:
    BIO_free_all(pub);
done:
    if (has_err)
        throw std::runtime_error("can't encrypt pass");
    return ans;
}



struct client_socket_t
{
    client_socket_t (uint32_t _addr, uint16_t _port, int _epoll_fd, std::string && _pass) :
        status(not_connected),
        fd(-1),
        epoll_fd(_epoll_fd),
        rsa_key(),
        pass(std::move(_pass)),
        encrypted_pass(),
        sender_password()
    {
        fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd == -1)
            throw std::runtime_error("can't create socket");
        
        sockaddr_in addr = {AF_INET, htons(_port), {htonl(_addr)}};
        
        int ret;
        ret = ::connect(fd, reinterpret_cast <sockaddr *> (&addr), sizeof(addr));
        status = in_connect;
        
        if (ret == -1)
        {
            if (errno == EINPROGRESS)
            {
                if (epoll_reg(epoll_fd, fd, EPOLLOUT))
                {
                    close(fd);
                    throw std::runtime_error("can't add to epoll");
                }
                return;
            }
            else
            {
                close(fd);
                switch (errno)
                {
                case ECONNREFUSED:
                    throw std::runtime_error("emm server exist?");
                case ENETUNREACH:
                    throw std::runtime_error("do you have network?");
                default:
                    perror("connect");
                    throw std::runtime_error("error on connect");
                }
            }
        }

        status = get_rsa_key;
        rsa_key.set(fd);
        if (epoll_reg(epoll_fd, fd, EPOLLIN))
            throw std::runtime_error("cant' add to epoll");
    }
    
    ~client_socket_t ()
    {
        if (fd != -1)
        {
            epoll_del(epoll_fd, fd);
            close(fd);
        }
    }
    
    int file_descriptor () const noexcept
    {
        return fd;
    }
    
    int reset_file_descriptor () noexcept
    {
        int _fd = fd;
        epoll_del(epoll_fd, fd);
        fd = -1;
        return _fd;
    }
    
    bool ready () const noexcept
    {
        return status == connected;
    }
    
    void write ();
    void read ();
    
private:    
    enum status_t
    {
        not_connected,
        in_connect,
        get_rsa_key,
        send_password,
        connected
    };
    
    void connect ();
    
    status_t status;
    int fd;
    int epoll_fd;

    recver_data_len rsa_key;
    
    std::string pass;
    std::string encrypted_pass;
    sender_data_len sender_password;
};


void client_socket_t::write ()
{
    if (status == in_connect)
    {
        connect();
        return;
    }
    if (status == send_password)
    {
        sender_password.write();
        if (sender_password.ready())
        {
            if (epoll_reg(epoll_fd, fd, EPOLLIN))
                throw std::runtime_error("cant' add to epoll");
            status = connected;
            sender_password.set(-1, std::string()); // free memory
        }
        return;
    }
}

void client_socket_t::read ()
{
    if (status == get_rsa_key)
    {
        rsa_key.read();
        if (rsa_key.ready())
        {
            status = send_password;
            encrypted_pass = enc_pass(pass, rsa_key.get());
            rsa_key.set(-1); // free memory
            sender_password.set(fd, std::move(encrypted_pass));
            if (epoll_reg(epoll_fd, fd, EPOLLOUT))
                throw std::runtime_error("cant' add to epoll");
        }
        return;
    }
    if (status == connected)
    {
        char buff[1024];
        ssize_t rb = ::read(fd, buff, sizeof(buff));
        if (rb != -1)
        {
            std::cout << std::string(buff, rb) << '\n';
        }
        return;
    }
}

void client_socket_t::connect ()
{
    int err = -1;
    socklen_t err_len = sizeof(err);

    int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len);
    if ((ret == -1) || (err != 0))
    {
        status = not_connected;
        if (err != 0)
            errno = err;
        close(fd);
        fd = -1;
        perror("connect event");
        throw std::runtime_error("error on connect (progress)");
    }
    
    status = get_rsa_key;
    rsa_key.set(fd);
    if (epoll_reg(epoll_fd, fd, EPOLLIN))
        throw std::runtime_error("cant' add to epoll");
}


int main ()
{
    std::string pass("00000000");

    try
    {
        epoll_wraper epoll_wrap;
        client_socket_t conn_sock(0x7f000001, 110, epoll_wrap.fd(), std::move(pass));
        
        bool exit = 0;
        while (!exit && !conn_sock.ready())
        {
            std::vector <std::pair <int, uint32_t> > events = epoll_wrap.wait();
            for (std::pair <int, uint32_t> const & event : events)
            {
                if (event.first == conn_sock.file_descriptor())
                {
                    if (event.second & EPOLLIN)
                        conn_sock.read();
                    if (event.second & EPOLLOUT)
                        conn_sock.write();
                }
            }
            
            for (std::pair <int, uint32_t> const & event : events)
            {
                static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                if (event.second & err_mask)
                {
                    try
                    {
                        epoll_wrap.unreg(event.first);
                    }
                    catch (...)
                    {}
            
                    if (event.first == conn_sock.file_descriptor())
                        exit = 1;
                }
            }
        }
        
        if (exit)
            return 1;
        
        interactive(conn_sock.reset_file_descriptor());
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {}
}

