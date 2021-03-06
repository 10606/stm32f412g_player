#include "epoll_wrapper.h"
#include "recver_data.h"
#include "sender_data.h"
#include "interactive.h"
#include "simple_socket.h"

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
#include <memory>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

std::string enc_pass (std::string_view password, std::string_view pub_key)
{
    std::unique_ptr <char []> encrypt;

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
        encrypt = std::make_unique <char []> (RSA_size(rsa));
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
                            reinterpret_cast <unsigned char *> (encrypt.get()),
                            rsa, 
                            RSA_PKCS1_OAEP_PADDING
                        )) == -1) 
    {
        std::cerr << "can't encrypt\n";
        goto rsa_free;
    }
    
    RSA_free(rsa);
    BIO_free_all(pub);
    return std::string(encrypt.get(), encrypt_len);

rsa_free:
    RSA_free(rsa);
bio_free:
    BIO_free_all(pub);
done:
    throw std::runtime_error("can't encrypt pass");
}



struct client_socket_t
{
    client_socket_t (in_addr_t _addr, uint16_t _port, epoll_wraper & _epoll) :
        status(not_connected),
        sock(-1),
        epoll(_epoll),
        rsa_key(),
        pass_pos(0),
        encrypted_pass(),
        sender_password()
    {
        sock = socket_t(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
        if (sock.fd() == -1)
            throw std::runtime_error("can't create socket");
        
        sockaddr_in addr = {AF_INET, htons(_port), {htonl(_addr)}};
        
        int ret;
        ret = ::connect(sock.fd(), reinterpret_cast <sockaddr *> (&addr), sizeof(addr));
        status = in_connect;
        
        if (ret == -1)
        {
            if (errno == EINPROGRESS)
            {
                try
                {
                    epoll.reg(sock.fd(), EPOLLOUT);
                }
                catch (...)
                {
                    close(sock.fd());
                    throw;
                }
                return;
            }
            else
            {
                close(sock.fd());
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
        rsa_key.set(sock);
        epoll.reg(sock.fd(), EPOLLIN);
    }
    
    ~client_socket_t ()
    {
        if (sock.fd() != -1)
        {
            try
            {
                epoll.unreg(sock.fd());
            }
            catch (...)
            {}
            close(sock.fd());
        }
    }
    
    client_socket_t (client_socket_t &&) = delete;
    client_socket_t (client_socket_t const &) = delete;
    client_socket_t & operator = (client_socket_t &&) = delete;
    client_socket_t & operator = (client_socket_t const &) = delete;
    
    int file_descriptor () const noexcept
    {
        return sock.fd();
    }
    
    int reset_file_descriptor () noexcept
    {
        int _fd = sock.fd();
        try
        {
            epoll.unreg(_fd);
        }
        catch (...)
        {}
        sock = socket_t(-1);
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
        read_password,
        send_password,
        connected
    };
    
    void connect ();
    
    status_t status;
    socket_t sock;
    epoll_wraper & epoll;

    recver_data_len <socket_t> rsa_key;
    
    char pass_buff[1024];
    size_t pass_pos;
    
    std::string encrypted_pass;
    sender_data_len <socket_t> sender_password;
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
            epoll.reg(sock.fd(), EPOLLIN);
            status = connected;
            sender_password.reset(); // free memory
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
            status = read_password;
            epoll.reg(STDIN_FILENO, EPOLLIN);
            epoll.reg(sock.fd(), 0);
        }
        return;
    }
    if (status == read_password)
    {
        ssize_t rb = ::read(STDIN_FILENO, pass_buff + pass_pos, 1);
        if (rb == -1)
        {
            if (errno != EINTR)
                throw std::runtime_error("read password");
            rb = 0;
        }
        if (rb == 0)
            return;
        if (pass_buff[pass_pos] == '\n')
        {
            status = send_password;
            encrypted_pass = enc_pass(std::string_view(pass_buff, pass_pos), rsa_key.get());
            rsa_key.reset(); // free memory
            sender_password.set(sock, std::move(encrypted_pass));
            epoll.reg(sock.fd(), EPOLLOUT);
            epoll.unreg(STDIN_FILENO);
        }
        else if (pass_pos + rb == sizeof(pass_buff))
            throw std::runtime_error("too long pass");
        pass_pos += rb;
        return;
    }
    if (status == connected)
    {
        return;
    }
}

void client_socket_t::connect ()
{
    int err = -1;
    socklen_t err_len = sizeof(err);

    int ret = getsockopt(sock.fd(), SOL_SOCKET, SO_ERROR, &err, &err_len);
    if ((ret == -1) || (err != 0))
    {
        status = not_connected;
        if (err != 0)
            errno = err;
        close(sock.fd());
        sock = socket_t(-1);
        perror("connect event");
        throw std::runtime_error("error on connect (progress)");
    }
    
    status = get_rsa_key;
    rsa_key.set(sock);
    epoll.reg(sock.fd(), EPOLLIN);
}


int main (int argc, char const * const * argv)
{
    try
    {
        in_addr addr{0x7f000001}; // 127.0.0.1
        if (argc > 1)
        {
            if (inet_aton(argv[1], &addr) == 0)
            {
                std::cout << "wrong address\n";
                return 1;
            }
        }

        epoll_wraper epoll_wrap;
        client_socket_t conn_sock(addr.s_addr, 750, epoll_wrap);
        
        bool exit = 0;
        while (!exit && !conn_sock.ready())
        {
            std::vector <epoll_wraper::e_event> events = epoll_wrap.wait();
            for (epoll_wraper::e_event const & event : events)
            {
                if ((event.fd == conn_sock.file_descriptor()) ||
                    (event.fd == STDIN_FILENO))
                {
                    if (event.mask & EPOLLIN)
                        conn_sock.read();
                    if (event.mask & EPOLLOUT)
                        conn_sock.write();
                }
            }
            
            for (epoll_wraper::e_event const & event : events)
            {
                static const uint32_t err_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                if (event.mask & err_mask)
                {
                    if ((event.fd == conn_sock.file_descriptor()) ||
                        (event.fd == STDIN_FILENO))
                        exit = 1;
                }
            }
        }
        
        if (exit)
            return 2;
        
        interactive(conn_sock.reset_file_descriptor());
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {}
}

