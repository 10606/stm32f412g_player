#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "usb_send.h"
#include "huffman_pic.h"
#include <algorithm>

namespace display
{

picture_info_t picture_info;

// it was lambda 
//  but i don't want to inline it
struct write_region_t
{
    write_region_t 
    (
        uint16_t const _x_pos,
        uint16_t const _y_pos,
        uint16_t const _x_size,
        uint16_t const _y_size, 
        uint32_t const _p_size,
        bool const _need_audio
    ) :
        x_pos(_x_pos),
        y_pos(_y_pos),
        x_size(_x_size),
        y_size(_y_size),
        p_size(_p_size),
        need_audio(_need_audio),
        
        line_cnt(0),
        p_old_size(0)
    {}
    
    __attribute__((noinline))
    bool operator () (uint16_t const * line);
    
private:
    uint16_t const x_pos; 
    uint16_t const y_pos;
    uint16_t const x_size;
    uint16_t const y_size; 
    uint32_t const p_size;
    bool const need_audio;
    
    uint32_t line_cnt;
    uint32_t p_old_size;
};

bool write_region_t::operator () (uint16_t const * line)
{
    if (line_cnt % p_size == 0) [[unlikely]]
    {
        if (need_audio)
            audio_ctl.audio_process();
        lcd_set_region(x_pos, y_pos + p_old_size, 
                       x_size, p_size);
        lcd_io_write_reg(ST7789H2_WRITE_RAM);
        p_old_size += p_size;
    }
    if (line_cnt == offsets::bg_color_on_picture.second) [[unlikely]]
        picture_info.color = line[offsets::bg_color_on_picture.first];
    
    for (size_t k = 0; k != x_size; ++k)
        lcd_io_write_data(line[k]);
    
    line_cnt++;
    return (line_cnt == y_size);
}

void display_picture 
(
    void const * addr, 
    uint16_t const x_pos, uint16_t const y_pos,
    uint16_t const x_size, uint16_t const y_size, 
    bool const need_audio
)
{
    scroller.reset();
    static constexpr uint32_t parts = 5;
    uint32_t const p_size = (y_size + parts - 1) / parts;
    
    huffman_header const * tree = static_cast <huffman_header const *> (addr);
    huffman_unp_pass_2 tree_p2(tree);
    uint8_t const * picture = static_cast <uint8_t const *> (addr) + sizeof(huffman_header);
    
    uint8_t state = inner_cnt - 1;
    uint32_t size = tree->sz;
    
    // wrong picture record
    {
        uint8_t * end_of_flash = reinterpret_cast <uint8_t *> (0x8100000);
        uint32_t sz_in_bytes = (size + 3) / 4;
        if ((picture + sz_in_bytes > end_of_flash) || (picture + sz_in_bytes < picture))
            size = (end_of_flash - picture) * 4;
    }
 
    typedef union 
    {
        uint8_t symbol[2 * 240 + 8];
        uint16_t pixel[240];
    } symbol_pixel_t;
    
    symbol_pixel_t line;
    uint32_t in_line_ptr = 0;
    uint32_t x_size_in_bytes = 2 * x_size;
    
    write_region_t write_region(x_pos, y_pos, x_size, y_size, p_size, need_audio);
    
    uint32_t ptr = 0;
    for (; ptr < size / 4; ++ptr)
    {
        uint8_t value = picture[ptr];
        for (size_t j = 0; j != 2; ++j)
        {
            huffman_unp_pass_2::state vertex = tree_p2.vertex[state][value % 16];
            line.symbol[in_line_ptr]     = vertex.values[0];
            line.symbol[in_line_ptr + 1] = vertex.values[1];
            in_line_ptr += vertex.count;
            state = vertex.next;
            value = value >> 4;
        }
        if (in_line_ptr >= x_size_in_bytes) [[unlikely]]
        {
            if (write_region(line.pixel)) [[unlikely]]
                return;
            memmove(line.symbol, line.symbol + x_size_in_bytes, in_line_ptr - x_size_in_bytes);
            in_line_ptr -= x_size_in_bytes;
        }
    }

    uint8_t value = picture[ptr];
    for (size_t j = 0; j != (size % 4); ++j)
    {
        huffman_unp_header::state vertex = tree->vertex[state][value % 4];
        line.symbol[in_line_ptr] = vertex.value;
        in_line_ptr += vertex.is_term;
        state = vertex.next;
        value = value >> 2;
        
        if (in_line_ptr == x_size_in_bytes)
        {
            if (write_region(line.pixel)) [[likely]]
                return;
            in_line_ptr = 0;
        }
    }
}


void start_image ()
{
    picture_info.update_start_pic_num();
    display_picture
    (
        picture_info.start_pic(), 
        0, 0, 
        240, 240, 
        0
    );
    sender.send_empty();
}

void song_image ()
{
    picture_info.update_song_pic_num();
    display_picture
    (
        picture_info.song_pic(), 
        0, display::offsets::picture, 
        240, 240 - display::offsets::picture, 
        1
    );
}

}

