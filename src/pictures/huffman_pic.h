#ifndef HUFFMAN_PIC_H
#define HUFFMAN_PIC_H

#include <stdint.h>
#include <stddef.h>

static const size_t inner_cnt = 85;

struct huffman_header
{
    // 4 child
    // store only inner

    struct state
    {
        uint8_t next : 7, is_term : 1;
        uint8_t value; // only if is_term
    };
    
    uint32_t sz; // by 2 bits
    state vertex[inner_cnt][4];
};

struct huffman_unp_header
{
    huffman_unp_header (huffman_header const * hh) :
        sz(hh->sz)
    {
        for (uint32_t i = 0; i != inner_cnt; ++i)
        {
            for (uint32_t j = 0; j != 4; ++j)
            {
                vertex[i][j] = hh->vertex[i][j];
            }
        }
    }

    struct state
    {
        state () = default;
    
        state (huffman_header::state st) :
            next(st.next),
            value(st.value),
            is_term(st.is_term)
        {}
        
        uint8_t next;
        uint8_t value;
        uint16_t is_term;
    };
    
    uint32_t sz; // by 2 bits
    state vertex[inner_cnt][4];
};

#endif

