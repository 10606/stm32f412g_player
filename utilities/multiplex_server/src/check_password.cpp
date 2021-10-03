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


