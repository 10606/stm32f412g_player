#include "display_picture.h"

#include "display_common.h"
#include "usb_send.h"

namespace display
{

bool write_region_t::operator () (uint16_t const * line)
{
    if (line_cnt % p_size == 0) [[unlikely]]
    {
        if (need_audio)
            audio_ctl.audio_process();
        lcd_set_region(pos.first, pos.second + p_old_size, 
                       size.first, p_size);
        lcd_io_write_reg(ST7789H2_WRITE_RAM);
        p_old_size += p_size;
    }
    if (line_cnt == offsets::bg_color_on_picture.second) [[unlikely]]
        picture_info.color = line[offsets::bg_color_on_picture.first];
    
    for (uint32_t k = 0; k != size.first; ++k)
        lcd_io_write_data(line[k]);
    
    line_cnt++;
    return (line_cnt == size.second);
}

picture_info_t picture_info;

void start_image ()
{
    picture_info.update_start_pic_num();
    display_picture
    (
        picture_info.start_pic(), 
        {0, 0}, 
        {240, 240}, 
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
        {0, display::offsets::picture}, 
        {240, 240 - display::offsets::picture}, 
        1
    );
}

}

