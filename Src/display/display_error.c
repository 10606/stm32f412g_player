#include "display.h"

#include "display_string.h"

void display_error (char const * msg)
{
    display_string_c(0, 152, msg, &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
}

