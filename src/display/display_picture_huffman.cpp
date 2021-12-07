#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "huffman_pic.h"
#include <algorithm>

namespace display
{

void display_picture 
(
    void const * addr, 
    std::pair <uint16_t, uint16_t> const pos,
    std::pair <uint16_t, uint16_t> const size,
    bool const need_audio
)
{
    scroller.reset();
    static constexpr uint32_t parts = 5;
    uint32_t const p_size = (size.second + parts - 1) / parts;
    
    huffman_header const * tree = static_cast <huffman_header const *> (addr);
    huffman_unp_pass_2 tree_p2(tree);
    uint8_t const * picture = static_cast <uint8_t const *> (addr) + sizeof(huffman_header);
    
    uint8_t state = inner_cnt - 1;
    uint32_t picture_size = tree->sz;
    
    // wrong picture record
    {
        uint8_t const * end_of_flash = reinterpret_cast <uint8_t const *> (0x8100000);
        uint32_t sz_in_bytes = (picture_size + 3) / 4;
        if ((picture + sz_in_bytes > end_of_flash) || (picture + sz_in_bytes < picture))
            picture_size = (end_of_flash - picture) * 4;
    }
 
    uint16_t pixel[240 + 4];
    uint8_t * symbol = reinterpret_cast <uint8_t *> (pixel);
    
    uint32_t in_line_ptr = 0;
    uint32_t x_size_in_bytes = 2 * size.first;
    
    write_region_t write_region(pos, size, p_size, need_audio);
    
    uint32_t ptr = 0;
    for (; ptr < picture_size / 4; ++ptr)
    {
        uint8_t value = picture[ptr];
        for (uint32_t j = 0; j != 2; ++j)
        {
            huffman_unp_pass_2::state vertex = tree_p2.vertex[state][value % 16];
            symbol[in_line_ptr]     = vertex.values[0];
            symbol[in_line_ptr + 1] = vertex.values[1];
            in_line_ptr += vertex.count;
            state = vertex.next;
            value = value >> 4;
        }
        if (in_line_ptr >= x_size_in_bytes) [[unlikely]]
        {
            if (write_region(pixel)) [[unlikely]]
                return;
            memcpy(symbol, symbol + x_size_in_bytes, in_line_ptr - x_size_in_bytes);
            in_line_ptr -= x_size_in_bytes;
        }
    }

    uint8_t value = picture[ptr];
    for (uint32_t j = 0; j != (picture_size % 4); ++j)
    {
        huffman_unp_header::state vertex = tree->vertex[state][value % 4];
        symbol[in_line_ptr] = vertex.value;
        in_line_ptr += vertex.is_term;
        state = vertex.next;
        value = value >> 2;
        
        if (in_line_ptr == x_size_in_bytes)
        {
            if (write_region(pixel)) [[likely]]
                return;
            in_line_ptr = 0;
        }
    }
}

}

