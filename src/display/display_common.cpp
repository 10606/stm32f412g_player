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
    char const * selected, 
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
            (selected[i] >> 1) ||
            
            (i == redraw_type.pos) ||
            (i == old_pos) ||
            ((i == border) && 
             (redraw_type.type == redraw_type_t::middle));
    */
}

constexpr uint8_t erase_bit (uint8_t value, uint8_t bit_index)
{
    uint8_t bit_mask = 1u << bit_index;
    value &= ~bit_mask; 
    bit_mask--;
    value += value & bit_mask;
    value >>= 1;
    return value;
}

constexpr uint8_t get_bits (uint8_t value, std::pair <uint8_t, uint8_t> bit_index)
{
    uint8_t f_bit = (value >> bit_index.first) & 1;
    uint8_t s_bit = (value >> bit_index.second) & 1;
    return f_bit + 2 * s_bit;
}

void display_lines 
(
    uint32_t i, 
    char const * line_0, 
    char const * line_1, 
    char const * selected, 
    uint16_t l0_text_color
)
{
    // 0
    // 1 - selected
    // 2 - playing
    // 3 - playing and selected
    
    // 4 - next
    // 5 - selected & next
    // 6 - playing & next
    // 7 - selected & playing & next

    color_t c_line_0;
    color_t c_line_1;
    
    auto & [text_color_line_0, back_color_line_0] = c_line_0;
    auto & [text_color_line_1, back_color_line_1] = c_line_1;

    char cur_selected = selected[i];
    
    if (cur_selected & (1 << 3)) // next == jump
    {
        cur_selected |= (1 << 2);
        cur_selected &= ~(1 << 3);
    }
    
    if (get_bits(cur_selected, {1, 2}) == 3) // playing & next
    {
        switch (cur_selected & 1)
        {
        case 0:
            back_color_line_0 = back_color_line_1 = lcd_color_white;
            text_color_line_0 = lcd_color_red;
            text_color_line_1 = lcd_color_black;
            break;
        case 1:
            back_color_line_0 = back_color_line_1 = lcd_color_blue;
            text_color_line_0 = lcd_color_green;
            text_color_line_1 = lcd_color_yellow;
            break;
        }
    }
    else
    {
        static const uint16_t text_color_lines [2][3] =  // nothing, playing, next
            {
                {lcd_color_blue,  lcd_color_red,   lcd_color_black},
                {lcd_color_white, lcd_color_green, lcd_color_yellow}
            };
        back_color_line_0 = back_color_line_1 = (cur_selected & 1)? lcd_color_blue : lcd_color_white;
        text_color_line_0 = text_color_line_1 = text_color_lines[cur_selected & 1][get_bits(cur_selected, {1, 2})];
        
    };

    display_string({offsets::x_padding, scroller.recalc_y(display::offsets::list + display::offsets::line * i)},
                line_0, &font_12, c_line_0);
    display_string({offsets::x_padding, scroller.recalc_y(display::offsets::list + display::offsets::line * i + display::offsets::in_line)}, 
                line_1, &font_12, c_line_1);
}

}

