#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "display_offsets.h"
#include "playlist_view.h"
#include "lcd_display.h"

namespace display
{

bool need_draw_line 
(
    uint32_t i, 
    uint32_t old_pos_playing,
    char const * selected, 
    redraw_type_t const & redraw_type,
    uint32_t view_cnt,
    bool force_redraw
);

void display_lines 
(
    uint32_t i, 
    char const * line_0, 
    char const * line_1, 
    char const * selected, 
    uint16_t l0_text_color
);


struct scroller_t
{
    scroller_t (uint16_t _top, uint16_t _bottom);
    
    void scroll (uint16_t value);
    void reset ();
    uint16_t recalc_y (uint16_t y);
    
private:
    uint16_t top;
    uint16_t bottom;
    uint16_t scrolled;
};

extern scroller_t scroller;


template <uint32_t view_cnt>
void scroll_text (redraw_type_t const & redraw_type)
{
    // disabled scroll 
    /*
    if (redraw_type.type == redraw_type_t::middle)
    {
        uint16_t scroll_value = display::offsets::line * ((redraw_type.direction == 0)? 1 : (view_cnt - 1));
        scroller.scroll(scroll_value);
    }
    */
}

template <uint32_t view_cnt>
void fill_borders ()
{
    fill_rect({0, offsets::headband}, {240, offsets::list - offsets::headband}, lcd_color_white); // top
    uint32_t y_pos_b = offsets::list + offsets::line * view_cnt;
    fill_rect({0, y_pos_b}, {240, 240 - y_pos_b}, lcd_color_white); // bottom
    fill_rect({0, offsets::headband}, {offsets::x_padding, 240 - offsets::headband}, lcd_color_white); // left
    uint32_t x_pos_r = (240 - offsets::x_padding) % font_12.Width;
    fill_rect({240 - x_pos_r, offsets::headband}, {x_pos_r, 240 - offsets::headband}, lcd_color_white); // right
}
    
}

#endif

