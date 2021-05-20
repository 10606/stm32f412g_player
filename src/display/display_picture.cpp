#include "display_picture.h"

#include "display_common.h"
#include "lcd_display.h"
#include "usb_send.h"

namespace display
{

picture_info_t picture_info;
const uint16_t * const picture_info_t::song = reinterpret_cast <uint16_t const *> (0x08080000);
const uint16_t * const picture_info_t::err = reinterpret_cast <uint16_t const *> (0x080c0000);
 

void start_image ()
{
    scroller.reset();
    picture_info.update_start_pic_num();
    draw_RGB_image(0, 0, 240, 240, picture_info.start_pic());
    send_empty();
}

void song_image (bool & need_redraw)
{
    scroller.reset();
    picture_info.update_song_pic_num();
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
}

}

