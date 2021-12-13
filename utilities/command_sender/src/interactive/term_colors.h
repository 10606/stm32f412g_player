#ifndef COLORS_H
#define COLORS_H

#include <vector>
#include <string>
#include <array>
#include <functional>

struct main_column_color 
{
    main_column_color 
    (
        std::array <std::string_view, 2> const & _back_color,
        std::array <std::array <std::string_view, 2>, 2> const & _playing_and_next,
        std::array <std::array <std::string_view, 3>, 2> const & _playing_or_next
    ) :
        back_color(_back_color),
        playing_and_next(_playing_and_next),
        playing_or_next(_playing_or_next)
    {}

    std::string operator () (size_t selected, size_t line)
    {
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
        
        // 4 - next
        // 5 - selected & next
        // 6 - playing & next
        // 7 - selected & playing & next
    
        if (selected & (1 << 3)) // next == jump
        {
            selected |= (1 << 2);
            selected &= ~(1 << 3);
        }
    
        std::string bc = std::string(back_color[selected & 1]);
        
        size_t playing_next = get_bits(selected, {1, 2});
        if (playing_next == 3) // playing & next
            return std::string(playing_and_next[line][selected & 1]) + ';' + bc;
        else
            return std::string(playing_or_next[selected & 1][playing_next]) + ';' + bc;
    };

private:
    constexpr size_t get_bits (size_t value, std::pair <size_t, size_t> bit_index)
    {
        size_t f_bit = (value >> bit_index.first) & 1;
        size_t s_bit = (value >> bit_index.second) & 1;
        return f_bit + 2 * s_bit;
    }

    std::array <std::string_view, 2> const back_color;
    std::array <std::array <std::string_view, 2>, 2> const playing_and_next; // [line][selected]
    std::array <std::array <std::string_view, 3>, 2> const playing_or_next;  // [selected][playing + next]
};

extern const std::array <std::function <std::string (size_t selected, size_t line)>, 2> main_columns;

struct colors
{
    static const std::array  //cmd
    <
        std::vector //current
        <
            std::string_view
        >,
        5
    > table;
};

struct color
{
    static const std::string_view defaul_color; 

    static const std::string_view white; 
    static const std::string_view red; 
    static const std::string_view dark_red; 
    static const std::string_view cyan;
    static const std::string_view green;
    static const std::string_view yellow;
    static const std::string_view black;
    static const std::string_view orange;
    static const std::string_view blue;
};

struct bg_color
{
    static const std::string_view black; 
    static const std::string_view blue; 
    static const std::string_view cyan; 
    static const std::string_view green; 
};

void print_color (std::string_view);

#endif

