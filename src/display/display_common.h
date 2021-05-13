#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "display_offsets.h"
#include "playlist_view.h"
#include "lcd_display.h"

namespace display
{

void display_lines 
(
    uint32_t i, 
    uint32_t old_pos_playing,
    char * line_0, 
    char * line_1, 
    char * selected, 
    redraw_type_t const & redraw_type, 
    uint32_t view_cnt, 
    uint16_t l0_text_color,
    bool force_redraw
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
    if (redraw_type.type == redraw_type_t::middle)
    {
        uint16_t scroll_value = display::offsets::line * ((redraw_type.direction == 0)? 1 : (view_cnt - 1));
        scroller.scroll(scroll_value);
    }
}

template <uint32_t view_cnt>
void fill_borders ()
{
    uint32_t y_pos = display::offsets::list + display::offsets::line * view_cnt;
    fill_rect(0, y_pos, 240, 240 - y_pos, lcd_color_white); // FIXME
}
    
}

#endif

