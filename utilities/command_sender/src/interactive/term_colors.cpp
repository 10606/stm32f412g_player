#include "term_colors.h"

#include <functional>
#include <iostream>

const std::string_view color::defaul_color = "00;00"; 

const std::string_view color::black = "00;30";
const std::string_view color::red = "00;31"; 
const std::string_view color::green = "00;32";
const std::string_view color::yellow = "00;33";
const std::string_view color::orange = "01;33";
const std::string_view color::blue = "00;34";
const std::string_view color::cyan = "00;36";
const std::string_view color::white = "00;37"; 

const std::string_view bg_color::blue = "44";
const std::string_view bg_color::cyan = "46";
const std::string_view bg_color::black = "40"; 

const std::array <std::function <std::string (size_t selected, size_t line)>, 2> main_columns = 
{
    main_column_color // not current
    (
        {bg_color::black, bg_color::blue},  // background
        {{ 
            {color::green, color::green},   // playing and next
            {color::orange, color::orange}  // [line][selected]
        }},
        {{
            {color::white, color::green, color::orange}, // [selected][playing + next]
            {color::white, color::green, color::orange}
        }}
    ),
    main_column_color // current
    (
        {bg_color::black, bg_color::cyan},  // background
        {{ 
            {color::green, color::red},     // playing and next
            {color::orange, color::blue}  // [line][selected]
        }},
        {{
            {color::white, color::green, color::orange}, // [selected][playing + next]
            {color::black, color::red,   color::blue}
        }}
    )
};

const std::vector  //cmd
<
    std::vector //current
    <
        std::string_view
    >
> colors::table = 
{
    {},
    
    { //cur_song_info
        {color::green}
    },
    
    {},
    {},
    { //volume_info
        { //not current
            color::white
        },
        { //current
            color::cyan
        }
    }
};

void print_color (std::string_view value)
{
    std::cout << "\033[" << value << "m";
}

