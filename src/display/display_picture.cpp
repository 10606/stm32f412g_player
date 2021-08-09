#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "usb_send.h"
#include "huffman_pic.h"
#include <algorithm>

namespace display
{

picture_info_t picture_info;

typedef union 
{
    uint8_t symbol[2 * 240];
    uint16_t pixel[240];
} symbol_pixel_t;

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
        bool const _need_audio,
        
        uint32_t & _line_cnt,
        uint32_t & _p_old_size,
        uint32_t & _in_line_ptr,
        symbol_pixel_t const & _line
    ) :
        x_pos(_x_pos),
        y_pos(_y_pos),
        x_size(_x_size),
        y_size(_y_size),
        p_size(_p_size),
        need_audio(_need_audio),
        
        line_cnt(_line_cnt),
        p_old_size(_p_old_size),
        in_line_ptr(_in_line_ptr),
        line(_line)
    {}
    
    __attribute__((noinline))
    bool operator () ();
    
private:
    uint16_t const x_pos; 
    uint16_t const y_pos;
    uint16_t const x_size;
    uint16_t const y_size; 
    uint32_t const p_size;
    bool const need_audio;
    
    uint32_t & line_cnt;
    uint32_t & p_old_size;
    uint32_t & in_line_ptr;
    symbol_pixel_t const & line;
};

bool write_region_t::operator () ()
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
    if (line_cnt == 6) [[unlikely]]
        picture_info.color = line.pixel[208];
    
    for (size_t k = 0; k != x_size; ++k)
        lcd_io_write_data(line.pixel[k]);
    
    line_cnt++;
    in_line_ptr = 0;
    if (line_cnt == y_size) [[unlikely]]
        return 1;
    return 0;
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
    uint32_t p_old_size = 0;
    
    huffman_unp_header tree(static_cast <huffman_header const *> (addr));
    uint8_t const * picture = static_cast <uint8_t const *> (addr) + sizeof(huffman_header);
    
    uint8_t state = inner_cnt - 1;
    
    // wrong picture record
    {
        uint8_t * end_of_flash = reinterpret_cast <uint8_t *> (0x8100000);
        uint32_t sz_in_bytes = (tree.sz + 3) / 4;
        if ((picture + sz_in_bytes > end_of_flash) || (picture + sz_in_bytes < picture))
            tree.sz = (end_of_flash - picture) * 4;
    }
 
    symbol_pixel_t line;
    uint32_t in_line_ptr = 0;
    uint32_t line_cnt = 0;
    uint32_t x_size_in_bytes = 2 * x_size;
    
    write_region_t write_region(x_pos, y_pos, x_size, y_size, p_size, need_audio,
                                line_cnt, p_old_size, in_line_ptr, line);
    
    uint32_t ptr = 0;
    for (; ptr < tree.sz / 4; ++ptr)
    {
        uint8_t value = picture[ptr];
        for (size_t j = 0; j != 4; ++j)
        {
            huffman_unp_header::state vertex = tree.vertex[state][value % 4];
            line.symbol[in_line_ptr] = vertex.value;
            in_line_ptr += vertex.is_term;
            state = vertex.next;
            value = value >> 2;
            
            if (in_line_ptr == x_size_in_bytes) [[unlikely]]
                if (write_region()) [[unlikely]]
                    return;
        }
    }

    uint8_t value = picture[ptr];
    for (size_t j = 0; j != (tree.sz % 4); ++j)
    {
        huffman_unp_header::state vertex = tree.vertex[state][value % 4];
        line.symbol[in_line_ptr] = vertex.value;
        in_line_ptr += vertex.is_term;
        state = vertex.next;
        value = value >> 2;
        
        if (in_line_ptr == x_size_in_bytes)
            if (write_region()) [[likely]]
                return;
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

