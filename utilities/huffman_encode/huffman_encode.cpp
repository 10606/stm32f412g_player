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

std::pair <huffman_header, rev_huffman_tree> 
build_tree (std::array <uint32_t, 256> freq)
{
    huffman_header ans;
    rev_huffman_tree rev;
    
    std::set <std::pair <uint32_t, uint16_t> > states; // <freq, index> (index < 256 -> term)
    for (size_t i = 0; i != 256; ++i)
        states.insert({freq[i], i});
    
    for (size_t i = 0; i != inner_cnt; ++i)
    {
        uint32_t sum = 0;
        for (size_t j = 0; j != 4; ++j)
        {
            std::set <std::pair <uint32_t, uint16_t> > :: iterator it = states.begin();
            sum += it->first;
            if (it->second < 256)
            {
                ans.vertex[i][j].is_term = 1;
                ans.vertex[i][j].next = inner_cnt - 1;
                ans.vertex[i][j].value = it->second;
            }
            else
            {
                ans.vertex[i][j].is_term = 0;
                ans.vertex[i][j].next = it->second - 256;
            }
            
            rev.states[it->second].prev = i + 256;
            rev.states[it->second].value = j;
            states.erase(it);
        }
        
        states.insert({sum, i + 256});
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

    std::pair <huffman_header, rev_huffman_tree> tree = build_tree(freq);
    in.clear();
    in.seekg(0);
    
    out.seekp(sizeof(tree.first));
    // need calc size, then write header
    uint8_t cur = 0;
    tree.first.sz = 0;
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
                rev_huffman_tree::state const & vertex = tree.second.states[state];
                bits.push_back(vertex.value);
                state = vertex.prev;
            }
            for (std::vector <uint8_t> :: reverse_iterator it = bits.rbegin(); it != bits.rend(); ++it)
            {
                cur += (*it) << (2 * (tree.first.sz % 4));
                tree.first.sz++;
                if (tree.first.sz % 4 == 0)
                {
                    out.put(static_cast <char> (cur));
                    cur = 0;
                }
            }
        }
    }
    if (tree.first.sz % 4 != 0)
        out.put(cur);
    out.seekp(0);
    out.write(reinterpret_cast <char *> (&tree.first), sizeof(tree.first));
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

