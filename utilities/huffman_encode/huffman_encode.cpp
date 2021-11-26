#include "huffman_pic.h"

#include <array>
#include <set>
#include <type_traits>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

struct rev_huffman_tree
{
    struct state 
    {
        uint16_t prev : 9, value : 2;
    };
    
    state states[256 + inner_cnt];
};

struct vertex_t
{
    vertex_t (uint32_t _freq = 0, uint16_t _index = 0) :
        freq(_freq),
        index(_index)
    {}

    friend std::strong_ordering operator <=> (vertex_t const & lhs, vertex_t const & rhs) = default;
    
    uint32_t freq;
    uint16_t index; // index < 256 -> term
};

std::pair <huffman_header, rev_huffman_tree> 
build_tree (std::array <uint32_t, 256> freq)
{
    huffman_header ans;
    rev_huffman_tree rev;
    
    std::set <vertex_t> states;
    for (size_t i = 0; i != 256; ++i)
        states.insert(vertex_t(freq[i], i));
    
    for (size_t i = 0; i != inner_cnt; ++i)
    {
        uint32_t sum = 0;
        for (size_t j = 0; j != 4; ++j)
        {
            std::set <vertex_t> :: iterator it = states.begin();
            sum += it->freq;
            if (it->index < 256)
            {
                ans.vertex[i][j].is_term = 1;
                ans.vertex[i][j].next = inner_cnt - 1;
                ans.vertex[i][j].value = it->index;
            }
            else
            {
                ans.vertex[i][j].is_term = 0;
                ans.vertex[i][j].next = it->index - 256;
            }
            
            rev.states[it->index].prev = i + 256;
            rev.states[it->index].value = j;
            states.erase(it);
        }
        
        states.insert(vertex_t(sum, i + 256));
    }
    
    return {ans, rev};
}

void compress (std::istream & in, std::ostream & out)
{
    std::array <uint32_t, 256> freq;
    std::fill(freq.begin(), freq.end(), 0);
    
    char buff[4096];
    while (in)
    {
        in.read(buff, std::extent_v <decltype(buff)>);
        size_t rb = in.gcount();
        for (size_t i = 0; i != rb; ++i)
            freq[static_cast <uint8_t> (buff[i])]++;
    }

    auto [huffman_tree, rev_huffman] = build_tree(freq);
    in.clear();
    in.seekg(0);
    
    out.seekp(sizeof(huffman_tree));
    // need calc size, then write header
    uint8_t cur = 0;
    huffman_tree.sz = 0;
    while (in)
    {
        in.read(buff, std::extent_v <decltype(buff)>);
        size_t rb = in.gcount();
        for (size_t i = 0; i != rb; ++i)
        {
            uint16_t state = static_cast <uint8_t> (buff[i]);
            std::vector <uint8_t> bits;
            while (state != 256 + inner_cnt - 1)
            {
                rev_huffman_tree::state const & vertex = rev_huffman.states[state];
                bits.push_back(vertex.value);
                state = vertex.prev;
            }
            for (std::vector <uint8_t> :: reverse_iterator it = bits.rbegin(); it != bits.rend(); ++it)
            {
                cur += (*it) << (2 * (huffman_tree.sz % 4));
                huffman_tree.sz++;
                if (huffman_tree.sz % 4 == 0)
                {
                    out.put(static_cast <char> (cur));
                    cur = 0;
                }
            }
        }
    }
    if (huffman_tree.sz % 4 != 0)
        out.put(cur);
    size_t cur_align = ((huffman_tree.sz + 3) / 4 + alignof(huffman_header) - 1) % alignof(huffman_header) + 1;
    for (size_t i = cur_align; i != alignof(huffman_header); ++i)
        out.put(0);
    out.seekp(0);
    out.write(reinterpret_cast <char *> (&huffman_tree), sizeof(huffman_tree));
}

void decompress (std::istream & in, std::ostream & out)
{
    huffman_header tree;
    size_t rb = 0;
    while (rb != sizeof(tree))
    {
        in.read(reinterpret_cast <char *> (&tree) + rb, sizeof(tree) - rb);
        rb += in.gcount();
    }
    
    uint8_t state = inner_cnt - 1;
    char buff[512];
    size_t ptr = 0, sz = 0;
    for (uint32_t i = 0; i != tree.sz; ptr++)
    {
        if (ptr == sz)
        {
            in.read(buff, std::extent_v <decltype(buff)>);
            sz = in.gcount();
            ptr = 0;
        }
        
        uint8_t value = buff[ptr];
        size_t cnt = ((tree.sz & ~3) > i)? 4 : (tree.sz % 4);
        for (size_t j = 0; j != cnt; ++j, ++i)
        {
            huffman_header::state const & vertex = tree.vertex[state][value % 4];
            if (vertex.is_term)
            {
                out.put(vertex.value);
            }
            state = vertex.next;
            value = value >> 2;
        }
    }
}

int main (int argc, char ** argv)
{
    if (argc < 3)
    {
        std::cout << argv[0] << " <src.rgb565> <dst.ch565>\n";
        return 1;
    }
    
    std::ifstream source(argv[1]);
    std::ofstream compressed(argv[2]);
    compress(source, compressed);
}

