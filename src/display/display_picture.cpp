#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "usb_send.h"
#include "huffman_pic.h"

namespace display
{

picture_info_t picture_info;

void display_picture 
(
    void const * addr, 
    uint16_t x_pos, uint16_t y_pos,
    uint16_t x_size, uint16_t y_size, 
    bool need_audio
)
{
    scroller.reset();
    static constexpr uint32_t parts = 5;
    uint32_t p_size = (y_size + parts - 1) / parts;
    uint32_t p_old_size = 0;
    
    huffman_header tree;
    memcpy(&tree, addr, sizeof(tree));
    uint8_t const * picture = static_cast <uint8_t const *> (addr) + sizeof(tree);
    
    uint8_t state = inner_cnt - 1;
    
    // wrong picture record
    {
        uint8_t * end_of_flash = reinterpret_cast <uint8_t *> (0x8100000);
        uint32_t sz_in_bytes = (tree.sz + 3) / 4;
        if ((picture + sz_in_bytes > end_of_flash) || (picture + sz_in_bytes < picture))
            tree.sz = (end_of_flash - picture) * 4;
    }
 
    union 
    {
        uint8_t symbol[2 * 240];
        uint16_t pixel[240];
    } line;
    uint32_t in_line_ptr = 0;
    uint32_t line_cnt = 0;
    uint32_t x_size_in_bytes = 2 * x_size;
    
    auto write_region = [&] () -> bool
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
    };
    
    uint32_t ptr = 0;
    for (; ptr < tree.sz / 4; ++ptr)
    {
        uint8_t value = picture[ptr];
        for (size_t j = 0; j != 4; ++j)
        {
            huffman_header::state const & vertex = tree.vertex[state][value % 4];
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
        huffman_header::state const & vertex = tree.vertex[state][value % 4];
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
    send_empty();
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

