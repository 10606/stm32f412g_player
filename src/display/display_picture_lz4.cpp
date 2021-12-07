#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"

namespace display
{

struct lz4_header
{
    uint8_t repeat : 4, size : 4;
};

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
    
    uint16_t pixel[240 * 5];
    char * buff = reinterpret_cast <char *> (pixel);
    uint16_t old_line[240];
    uint32_t tail = 0;
    
    char const * picture = static_cast <char const *> (addr);
    uint32_t picture_size_in_bytes = size.first * size.second * sizeof(uint16_t);
    uint32_t position = 0;
    
    write_region_t write_region(pos, size, p_size, need_audio);
    
    while (1)
    {
        lz4_header cur;
        memcpy(&cur, picture, sizeof(cur));
        uint32_t cur_size = cur.size;
        uint32_t cur_repeat = cur.repeat;
        picture++;
    
        for (uint32_t j = 0; j != cur_size; ++j)
            buff[position + j] = *(picture++);
        
        for (uint32_t i = 1; i < cur_repeat; ++i)
            for (uint32_t j = 0; j != cur_size; ++j)
                buff[position + i * cur_size + j] = buff[position + j];
        position += cur_size * cur_repeat;
        
        if ((position == sizeof(pixel)) ||
            (position == picture_size_in_bytes)) [[unlikely]]
        {
            uint32_t pixel_end = position / sizeof(uint16_t);
            
            uint32_t index;
            if (tail) 
            {
                index = size.first - tail;
                memcpy(old_line + tail, pixel, index);
                if (write_region(old_line)) [[unlikely]]
                    return;
            }
            else
                index = 0;

            for (; index + size.first <= pixel_end; index += size.first)
                if (write_region(pixel + index)) [[unlikely]]
                    return;
            
            if (index + size.first != pixel_end)
            {
                tail = pixel_end - index;
                memcpy(old_line, pixel + index, tail);
            }
            else
                tail = 0;
            
            picture_size_in_bytes -= position;
            position = 0;

            if (picture_size_in_bytes == 0)
                return;
        }
    }
}

}

