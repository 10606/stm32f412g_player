#ifndef UTF8_AUTOMAT_H
#define UTF8_AUTOMAT_H

#include <stdint.h>

struct utf8_automat
{
    uint8_t symbols;
    uint32_t ans;
    
    utf8_automat () :
        symbols(0),
        ans(0)
    {}
    
    enum state_t
    {
        none,
        ready,
        err,
    };
    
    state_t next (char _c);
    
    uint32_t get_ans ()
    {
        return ans;
    }
};


#endif

