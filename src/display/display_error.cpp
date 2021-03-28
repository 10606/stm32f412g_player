#include "display.h"

#include "lcd_display.h"

void display_error (char const * msg)
{
    display_string_c(0, 152, msg, &font_16, lcd_color_white, lcd_color_red);
}
