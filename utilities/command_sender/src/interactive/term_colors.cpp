#include "term_colors.h"

#include <functional>
#include <iostream>

const std::string_view color::defaul_color = "00;00"; 

const std::string_view color::black = "00;30";
const std::string_view color::red = "00;31"; 
const std::string_view color::dark_red = "01;30"; 
const std::string_view color::green = "00;32";
const std::string_view color::yellow = "00;33";
const std::string_view color::orange = "01;33";
const std::string_view color::blue = "00;34";
const std::string_view color::cyan = "00;36";
const std::string_view color::white = "00;37"; 

const std::string_view bg_color::blue = "44";
const std::string_view bg_color::cyan = "46";
const std::string_view bg_color::black = "40"; 
const std::string_view bg_color::green = "42"; 

const std::array <std::function <std::string (size_t selected, size_t line)>, 2> main_columns = 
{
    main_column_color // not current
    (
        {bg_color::black, bg_color::blue},  // [selected]
        {{ 
            {color::green,  color::green,  color::green,  color::green},  // (playing | jmp) and next
            {color::yellow, color::yellow, color::orange, color::orange}  // [line][playing + selected]
        }},
        {{
            {color::white, color::yellow, color::orange, color::orange, color::green}, // [selected][next + playing + jmp]
            {color::white, color::yellow, color::orange, color::orange, color::green}
        }}
    ),
    main_column_color // current
    (
        {bg_color::black, bg_color::green},     // [selected]
        {{ 
            {color::green,  color::dark_red, color::green,  color::dark_red},   // (playing | jmp) and next
            {color::yellow, color::blue,     color::orange, color::blue}        // [line][playing + selected]
        }},
        {{
            {color::white, color::yellow, color::orange, color::orange, color::green   }, // [selected][next + playing + jmp]
            {color::black, color::blue,   color::blue  , color::blue  , color::dark_red}
        }}
    )
};

const std::array  //cmd
<
    std::vector //current
    <
        std::string_view
    >,
    5
> colors::table = 
{{
    {},
    
    //cur_song_info
    {color::green},
    
    {},
    {},
    
    //volume_info
    {color::white, color::cyan}
}};

void print_color (std::string_view value)
{
    std::cout << "\033[" << value << "m";
}

