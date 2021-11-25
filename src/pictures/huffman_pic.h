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

struct huffman_unp_pass_2
{

    huffman_unp_pass_2 () = default;
    
    huffman_unp_pass_2 (huffman_header const * hh) :
        sz(hh->sz)
    {
        for (uint32_t i = 0; i != inner_cnt; ++i)
        {
            for (uint32_t j = 0; j != 16; ++j)
            {
                huffman_unp_header::state s0 = hh->vertex[i][j & 0b11];
                huffman_unp_header::state s1 = hh->vertex[s0.next][j >> 2];
                uint8_t values[2];
                values[0] = s0.value;
                values[s0.is_term] = s1.value;
                vertex[i][j] = state(s1.next, values, s0.is_term + s1.is_term);
            }
        }
    }

    struct state
    {
        state () = default;
        
        state (uint8_t _next, uint8_t * _values, uint16_t _count) :
            next(_next),
            values{_values[0], _values[1]},
            count(_count)
        {}
    
        uint8_t next;
        uint8_t values[2];
        uint16_t count;
    };
    
    uint32_t sz; // by 2 bits
    state vertex[inner_cnt][16];
};

#endif

