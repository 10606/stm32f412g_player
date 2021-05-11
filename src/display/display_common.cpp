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
    
    scroller_t scroller(display::offsets::list, display::offsets::line * plb_view_cnt);
}

