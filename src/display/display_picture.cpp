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
    bool need_audio, bool & need_redraw
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
    uint16_t pixel = 0;
    uint32_t pixel_cnt = 0; // in half (2 -> 1 pixel)
    uint32_t ptr = 0;
    
    uint8_t * end_of_flash = reinterpret_cast <uint8_t *> (0x8100000);
    if ((picture + tree.sz > end_of_flash) || (picture + tree.sz < picture))
        tree.sz = end_of_flash - picture;
    
    for (uint32_t i = 0; i != tree.sz; ++ptr)
    {
        uint8_t value = picture[ptr];
        size_t cnt = ((tree.sz & ~3) > i)? 4 : (tree.sz % 4);
        for (size_t j = 0; j != cnt; ++j, ++i)
        {
            huffman_header::state const & vertex = tree.vertex[state][value % 4];
            if (vertex.is_term) [[unlikely]]
            {
                if (pixel_cnt % 2)
                {
                    pixel += (vertex.value << 8);
                    lcd_io_write_data(pixel);
                    if (pixel_cnt + 1 == 2 * (6 * static_cast <uint32_t> (x_size) + 208)) [[unlikely]]
                        picture_info.color = pixel;
                    
                    pixel = 0;
                }
                else
                {
                    if (pixel_cnt % (x_size * p_size * 2) == 0) [[unlikely]]
                    {
                        if (need_audio)
                            audio_ctl.audio_process(need_redraw);
                        lcd_set_region(x_pos, y_pos + p_old_size, 
                                       x_size, p_size);
                        lcd_io_write_reg(ST7789H2_WRITE_RAM);
                        p_old_size += p_size;
                    }
                    
                    if (pixel_cnt == x_size * y_size * 2) [[unlikely]]
                        return;
                    
                    pixel = vertex.value;
                }
                ++pixel_cnt;
            }
            state = vertex.next;
            value = value >> 2;
        }
    }
}


void start_image ()
{
    picture_info.update_start_pic_num();
    bool need_redraw;
    display_picture(picture_info.start_pic(), 0, 0, 240, 240, 0, need_redraw);
    //draw_RGB_image(0, 0, 240, 240, picture_info.start_pic());
    send_empty();
}

void song_image (bool & need_redraw)
{
    picture_info.update_song_pic_num();
    display_picture
    (
        picture_info.song_pic(), 
        0, display::offsets::picture, 
        240, 240 - display::offsets::picture, 
        1, need_redraw
    );
    /*
    static constexpr uint32_t parts = 5;
    uint32_t p_size = (240 - display::offsets::picture + parts - 1) / parts;
    uint32_t p_old_size = p_size;
    
    uint16_t const * picture = picture_info.song_pic(); 
    for (uint32_t part = 0; part != parts; ++part)
    {
        if (part + 1 == parts)
            p_size = (240 - display::offsets::picture) - (parts - 1) * p_old_size;
        draw_RGB_image(0, display::offsets::picture + p_old_size * part, 
                       240, p_size, 
                       picture + 240 * part * p_old_size);
        audio_ctl.audio_process(need_redraw);
    }
    */
}

}

