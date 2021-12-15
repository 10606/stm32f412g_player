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
        std::array <std::array <std::string_view, 4>, 2> const & _playing_and_next,
        std::array <std::array <std::string_view, 5>, 2> const & _playing_or_next
    ) :
        back_color(_back_color),
        playing_and_next(_playing_and_next),
        playing_or_next(_playing_or_next)
    {}

    std::string operator () (size_t selected, size_t line)
    {
        //bits of selected:
        // jmp  next  playing  selected
    
        std::string bc = std::string(back_color[selected & 1]);
        
        size_t playing_next_jmp = get_bits(selected, {3, 2, 1});
        if ((playing_next_jmp & 4) && (playing_next_jmp ^ 4)) // playing & (next | jmp)
            return std::string(playing_and_next[line][get_bits(selected, {0, 2})]) + ';' + bc;
        else
            return std::string(playing_or_next[selected & 1][playing_next_jmp]) + ';' + bc;
    };

private:
    constexpr size_t get_bits (size_t value, std::initializer_list <size_t> const & bit_index)
    {
        size_t ans = 0;
        size_t i = 0;
        for (size_t index : bit_index)
        {
            ans += ((value >> index) & 1) << i;
            ++i;
        }
        return ans;
    }

    std::array <std::string_view, 2> const back_color;
    std::array <std::array <std::string_view, 4>, 2> const playing_and_next; // [line][selected]
    std::array <std::array <std::string_view, 5>, 2> const playing_or_next;  // [selected][playing + next]
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

