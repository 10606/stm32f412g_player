#include "term_display.h"

#include "char_reconvert.h"
#include "common_types.h"
#include "term_colors.h"
#include "calc_utf8_len.h"
#include <optional>
#include <stdint.h>
#include <array>
#include <cstring>
#include <vector>

inline void set_cursor (std::pair <size_t, size_t> pos)
{
    std::cout << "\033[" << pos.second << ";" << pos.first << "H";
}


constexpr inline std::pair <size_t, size_t> 
operator +
(
    std::pair <size_t, size_t> const & a, 
    std::pair <size_t, size_t> const & b
) noexcept
{
    return std::make_pair (a.first + b.first, a.second + b.second);
}

constexpr inline std::pair <size_t, size_t> 
operator *
(
    std::pair <size_t, size_t> const & a, 
    size_t b
) noexcept
{
    return std::make_pair (a.first * b, a.second * b);
}

constexpr inline std::pair <size_t, size_t> 
operator *
(
    size_t b,
    std::pair <size_t, size_t> const & a
) noexcept
{
    return std::make_pair (a.first * b, a.second * b);
}

struct base
{
    typedef std::pair <size_t, size_t> coord;
    static const size_t left_col = std::extent <decltype(pl_list_info_t::name)> :: value;
    
    static const constexpr coord fake[] = {{0, 0}, {0, 0}};
    
    static const constexpr coord cur_song[] = {{10, 1}, {10, 2}};

    static const constexpr coord disp_song[] = {{left_col + 2, 4}, {left_col + 2, 5}};
    static const constexpr coord disp_song_a{0, 2};

    static const constexpr coord pl_list[] = {{0, 4}, {0, 5}};
    static const constexpr coord pl_list_a{0, 2};
    
    static const constexpr coord volume[] = {{left_col + 2 + 5, 1}, {left_col + 2 + 5, 2}};

    static const constexpr coord search[] = {{pl_list[1] + 7 * pl_list_a}, {pl_list[0] + 8 * pl_list_a}};
    
    static const constexpr coord * const addition[5] = 
    {
        fake,
        cur_song,
        disp_song,
        pl_list,
        volume,
    };

    static const constexpr coord repeat_counter = {left_col + 20, 2};
};

inline void set_color (size_t current, size_t cmd, size_t line, size_t s = 0)
{
    if (cmd == displayed_song_info ||
        cmd == pl_list_info)
    {
        if (current > main_columns.size())
            return;
        if (line > 1)
            return;
        print_color(main_columns[current](s, line));
        return;
    }
    
    if (cmd >= colors::table.size())
        return;
    if (current >= colors::table[cmd].size())
        return;
    print_color(colors::table[cmd][current]);
}

bool is_spaces (std::string const & str) noexcept
{
    return std::all_of(str.begin(), str.end(), 
        [] (char c) -> bool 
        { return std::isspace(c) || (c == 0); });
}

template <typename T>
void display_lines (T const & data, bool is_my_state, std::pair <size_t, size_t> base_multiplier = {0, 0})
{
    if (data.cmd >= std::extent <decltype(base::addition)>::value)
        return;
    std::pair <size_t, size_t> base_pos = get_pos(data) * base_multiplier;
    
    set_cursor(base::addition[data.cmd][0] + base_pos);
    set_color(is_my_state, data.cmd, 0, get_selected(data));
    std::cout << char_reconvert(std::string_view(data.line_0, std::extent <decltype(T::line_0)>::value));
    
    set_cursor(base::addition[data.cmd][1] + base_pos);
    set_color(is_my_state, data.cmd, 1, get_selected(data));
    std::cout << char_reconvert(std::string_view(data.line_1, std::extent <decltype(T::line_1)>::value));
    
    print_color(color::defaul_color);
}

template <typename T>
std::optional <T> clone (std::deque <char> & data)
{
    size_t my_size = sizeof(T);
    if (my_size > data.size())
        return std::nullopt;
    T my_data;
    for (size_t i = 0; i != my_size; ++i)
        *(reinterpret_cast <char *> (&my_data) + i) = data[i];
    data.erase(data.begin(), data.begin() + my_size);
    return my_data;
}

void set_cursor_pos_search (std::array <std::string, 2> const & value, size_t index)
{
    std::pair <size_t, size_t> pos = base::search[index];
    pos.first += calc_utf8_len(value[index]) + 1;
    set_cursor(pos);
    std::cout.flush();
}

void display_search (std::array <std::string, 2> const & value)
{
    static size_t prev_size[2] = {0, 0};
    static std::string_view line_color[2] = {color::green, color::yellow};
    
    for (size_t i = 0; i != 2; ++i)
    {
        set_cursor(base::search[i]);
        print_color(line_color[i]);
        std::cout << value[i];
        for (size_t j = value[i].size(); j < prev_size[i] + 10; ++j)
            std::cout  << ' ';
        prev_size[i] = value[i].size();
    }
    std::cout.flush();
}

void display_number (position_t value)
{
    set_cursor(base::search[0]);
    if (value != 0)
    {
        print_color(color::green);
        std::cout << value;
    }
    for (size_t i = 0; i != 10; ++i)
        std::cout << ' ';
    std::cout.flush();
}

void extract (std::deque <char> & data, state_t & state)
{
    if (data.empty())
        return;
    
    while (!data.empty())
    {
        switch (data.front())
        {
            case cur_song_info:
                if (std::optional <cur_song_info_t> song_data = clone <cur_song_info_t> (data))
                    display_lines(song_data.value(), 0);
                else
                    goto end_for;
                break;

            case displayed_song_info:
                if (std::optional <displayed_song_info_t> song_data = clone <displayed_song_info_t> (data))
                    display_lines(song_data.value(), state == state_t::playlist, base::disp_song_a);
                else
                    goto end_for;
                break;

            case pl_list_info:
                if (std::optional <pl_list_info_t> pl_list_data = clone <pl_list_info_t> (data))
                {
                    bool is_my_state = state == state_t::pl_list;
                    std::pair <size_t, size_t> base_pos = pl_list_data->pos * base::pl_list_a;
                    
                    set_cursor(base::pl_list[0] + base_pos);
                    set_color(is_my_state, pl_list_data->cmd, 0, pl_list_data->selected);
                    std::string line_0(pl_list_data->name, sizeof(pl_list_info_t::name));
                    std::cout << char_reconvert(line_0);
                    
                    set_cursor(base::pl_list[1] + base_pos);
                    set_color(is_my_state, pl_list_data->cmd, 1, pl_list_data->selected);
                    if (!is_spaces(line_0))
                        std::cout << "  -------------------------------";
                    else
                        std::cout << "                                 ";
                    print_color(color::defaul_color);
                }
                else
                    goto end_for;
                break;

            case volume_info:
                if (std::optional <volume_info_t> volume_data = clone <volume_info_t> (data))
                {
                    display_lines(volume_data.value(), state == state_t::song);
                    
                    set_cursor(base::repeat_counter);
                    print_color(color::defaul_color);
                    if (volume_data.value().repeat_counter)
                        std::cout << volume_data.value().repeat_counter << "   ";
                    else
                        std::cout << "    ";
                }
                else
                    goto end_for;
                break;

            case state_info:
                if (std::optional <state_info_t> state_info = clone <state_info_t> (data))
                    state = state_info->state;
                else
                    goto end_for;
                break;

            default:
                data.pop_front();
                break;
        }
    }
    end_for:
    std::cout.flush();
}


