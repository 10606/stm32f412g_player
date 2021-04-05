#include "term_display.h"

#include "common_types.h"
#include "term_colors.h"
#include <optional>
#include <stdint.h>
#include <cstring>
#include <vector>

inline void set_cursor (std::pair <size_t, size_t> pos)
{
    std::cout << "\033[" << pos.second << ";" << pos.first << "H";
}


inline std::pair <size_t, size_t> 
operator +
(
    std::pair <size_t, size_t> const & a, 
    std::pair <size_t, size_t> const & b
)
{
    return std::make_pair (a.first + b.first, a.second + b.second);
}

inline std::pair <size_t, size_t> 
operator *
(
    std::pair <size_t, size_t> const & a, 
    size_t b
)
{
    return std::make_pair (a.first * b, a.second * b);
}

inline std::pair <size_t, size_t> 
operator *
(
    size_t b,
    std::pair <size_t, size_t> const & a
)
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

    static const constexpr coord * const addition[5] = 
    {
        fake,
        cur_song,
        disp_song,
        pl_list,
        volume,
    };
};

inline void set_color (size_t current, size_t cmd, size_t line, size_t s = 0)
{
    if (cmd >= colors::table.size())
        return;
    if (line >= colors::table[cmd].size())
        return;
    if (current >= colors::table[cmd][line].size())
        return;
    if (s >= colors::table[cmd][line][current].size())
        return;
    std::cout << colors::table[cmd][line][current][s];
}

bool is_spaces (std::string const & str)
{
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if ((!std::isspace(*it)) && (*it != 0))
            return 0;
    }
    return 1;
}

template <typename T>
void display_lines (T const & data, bool is_my_state, std::pair <size_t, size_t> base_multiplier = {0, 0})
{
    if (data.cmd >= std::extent <decltype(base::addition)>::value)
        return;
    std::pair <size_t, size_t> base_pos = get_pos(data) * base_multiplier;
    
    set_cursor(base::addition[data.cmd][0] + base_pos);
    set_color(is_my_state, data.cmd, 0, get_selected(data));
    std::cout << std::string_view(data.line_0, std::extent <decltype(T::line_0)>::value);
    
    set_cursor(base::addition[data.cmd][1] + base_pos);
    set_color(is_my_state, data.cmd, 1, get_selected(data));
    std::cout << std::string_view(data.line_1, std::extent <decltype(T::line_1)>::value);
    
    std::cout << color::defaul_color;
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
                    std::cout << line_0;
                    set_cursor(base::pl_list[1] + base_pos);
                    set_color(is_my_state, pl_list_data->cmd, 1, pl_list_data->selected);
                    if (!is_spaces(line_0))
                        std::cout << "  -------------------------------  ";
                    else
                        std::cout << "                                   ";
                    std::cout << color::defaul_color;
                }
                else
                    goto end_for;
                break;

            case volume_info:
                if (std::optional <volume_info_t> volume_data = clone <volume_info_t> (data))
                    display_lines(volume_data.value(), state == state_t::song);
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


