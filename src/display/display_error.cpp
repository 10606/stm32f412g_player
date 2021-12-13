#include "display_error.h"

#include "lcd_display.h"

namespace display
{

void error (char const * msg)
{
    display_string(0, 152, msg, &font_16, {lcd_color_white, lcd_color_red});
}

}

