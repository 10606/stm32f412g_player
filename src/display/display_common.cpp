#include "display_common.h"

#include "lcd_display.h"
#include "stm32f4xx_hal_gpio.h"
#include "display_init.h"
#include "pl_list.h"

namespace display 
{

scroller_t::scroller_t (uint16_t _top, uint16_t _bottom) :
    top(_top),
    bottom(_bottom),
    scrolled(0)
{
    lcd_set_scroll_region(top, bottom);
}

void scroller_t::scroll (uint16_t value)
{
    scrolled = (scrolled + value) % bottom;
    uint16_t vsp = top + scrolled;
    lcd_set_scroll_region(top, bottom);
    lcd_scroll(vsp);
}

void scroller_t::reset ()
{
    scrolled = 0;
    lcd_set_scroll_region(top, bottom);
    lcd_scroll(top);
}

uint16_t scroller_t::recalc_y (uint16_t y)
{
    if (y < top)
        return y;
    if (y > top + bottom)
        return y;
    return ((y - top + /*bottom -*/ scrolled) % bottom) + top;
}

scroller_t scroller(display::offsets::list, display::offsets::line * pl_list::view_cnt);
    
 
bool need_draw_line 
(
    uint32_t i, 
    uint32_t old_pos_playing,
    char * selected, 
    redraw_type_t const & redraw_type,
    uint32_t view_cnt,
    bool force_redraw
)
{
    // disabled scroll
    return 1; 
    /*
    uint32_t old_pos = (redraw_type.pos + view_cnt + (redraw_type.direction? 1 : -1)) % view_cnt;
    uint32_t border = redraw_type.direction? 0 : (view_cnt - 1);
    
    return  force_redraw ||
            (redraw_type.type == redraw_type_t::not_easy) ||
            
            (i == old_pos_playing) || // playing
            (selected[i] & 2) ||
            
            (i == redraw_type.pos) ||
            (i == old_pos) ||
            ((i == border) && 
             (redraw_type.type == redraw_type_t::middle));
    */
}
    
void display_lines 
(
    uint32_t i, 
    char * line_0, 
    char * line_1, 
    char * selected, 
    uint16_t l0_text_color
)
{
    uint16_t back_color_line_0, text_color_line_0;
    uint16_t back_color_line_1, text_color_line_1;
    
    switch (selected[i])
    {
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
        case 3:
            back_color_line_0 = lcd_color_blue;
            text_color_line_0 = l0_text_color;
            back_color_line_1 = lcd_color_blue;
            text_color_line_1 = lcd_color_green;
            break;
        case 2:
            back_color_line_1 = back_color_line_0 = lcd_color_white;
            text_color_line_1 = text_color_line_0 = lcd_color_red;
            break;
        case 1:
            back_color_line_1 = back_color_line_0 = lcd_color_blue;
            text_color_line_1 = text_color_line_0 = lcd_color_white;
            break;
        default:
            back_color_line_1 = back_color_line_0 = lcd_color_white;
            text_color_line_1 = text_color_line_0 = lcd_color_blue;
    }

    color_t c_line_0 = {text_color_line_0, back_color_line_0};
    color_t c_line_1 = {text_color_line_1, back_color_line_1};
    
    display_string(4, scroller.recalc_y(display::offsets::list + display::offsets::line * i),
                line_0, &font_12, &c_line_0);
    display_string(4, scroller.recalc_y(display::offsets::list + display::offsets::line * i + display::offsets::in_line), 
                line_1, &font_12, &c_line_1);
}

}

