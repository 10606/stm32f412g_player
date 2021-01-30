#include "display.h"

#include "colors.h"
#include <stdint.h>
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
    
    static const constexpr coord cur_song_0{10, 1};
    static const constexpr coord cur_song_1{10, 2};

    static const constexpr coord disp_song_0{name_offset + pl_name_sz + count_offset + 4 + 2, 4};
    static const constexpr coord disp_song_1{name_offset + pl_name_sz + count_offset + 4 + 2, 5};
    static const constexpr coord disp_song_a{0, 2};

    static const constexpr coord pl_list_0{0, 4};
    static const constexpr coord pl_list_1{0, 5};
    static const constexpr coord pl_list_a{0, 2};
    
    static const constexpr coord volume_0{name_offset + pl_name_sz + count_offset + 4 + 2 + 5, 1};
    static const constexpr coord volume_1{name_offset + pl_name_sz + count_offset + 4 + 2 + 5, 2};
};

inline void set_color (bool current, char cmd, bool line, char s = 0)
{
    if ((cmd > 0x05) || (cmd < 0))
        return;
    if ((s > 3) || (s < 0))
        return;

    if (cmd == cur_song_info)
    {
        current = 0;
        line = 0;
        s = 0;
    }
    if (cmd == displayed_song_info)
    {
    }
    if (cmd == pl_list_info)
    {
        //line = 0;
    }
    if (cmd == volume_info)
    {
        s = 0;
    }
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

struct msg_sizes
{
    static uint32_t const cur_song = song_name_sz + 1 + group_name_sz + 1 + 1;
    static uint32_t const displayed_song = name_offset + group_name_sz + 1 + name_offset + song_name_sz + 1 + 3;
    static uint32_t const pl_list = name_offset + pl_name_sz + count_offset + 4 + 3;
    static uint32_t const volume = volume_width + volume_width + 1;
};

void extract (std::string & data, size_t & state)
{
    if (data.empty())
        return;
    
    size_t pos;
    for (pos = 0; pos != data.size(); )
    {
        /*
        std::cout << ((data[pos] == 0x05)? '!' : data[pos]);
        pos++;
        continue;
        */
        
        switch (data[pos])
        {
            case cur_song_info:
                if (pos + msg_sizes::cur_song > data.size())
                    goto end_for;
                set_cursor(base::cur_song_0);
                set_color(0, cur_song_info, 0);
                std::cout << data.substr(pos + 1 + song_name_sz + 1, group_name_sz + 1);
                set_cursor(base::cur_song_1);
                set_color(0, cur_song_info, 1);
                std::cout << data.substr(pos + 1, song_name_sz + 1);
                std::cout << color::defaul_color;
                pos += msg_sizes::cur_song;
                break;

            case displayed_song_info:
                if (pos + msg_sizes::displayed_song > data.size())
                    goto end_for;
                //std::cout << ((data[pos + 1] & 1)? 's' : ' ') << " " << static_cast <size_t> (data[pos + 2]) << "\n";
                set_cursor(base::disp_song_0 + static_cast <size_t> (data[pos + 2]) * base::disp_song_a);
                set_color(state == 1, displayed_song_info, 0, data[pos + 1]);
                std::cout << data.substr(pos + 3, name_offset + group_name_sz + 1);
                set_cursor(base::disp_song_1 + static_cast <size_t> (data[pos + 2]) * base::disp_song_a);
                set_color(state == 1, displayed_song_info, 1, data[pos + 1]);
                std::cout << data.substr(pos + 3 + name_offset + group_name_sz + 1, name_offset + song_name_sz + 1);
                std::cout << color::defaul_color;
                pos += msg_sizes::displayed_song;
                break;

            case pl_list_info:
                if (pos + msg_sizes::pl_list > data.size())
                    goto end_for;
                //std::cout << (data[pos + 1]? 's' : ' ') << " " << data[pos + 2] << "\n";
                set_cursor(base::pl_list_0 + static_cast <size_t> (data[pos + 2]) * base::pl_list_a);
                set_color(state == 0, pl_list_info, 0, data[pos + 1]);
                std::cout << data.substr(pos + 3, name_offset + pl_name_sz + count_offset + 4);
                if (!is_spaces(data.substr(pos + 3, name_offset + pl_name_sz + count_offset + 4)))
                {
                    set_cursor(base::pl_list_1 + static_cast <size_t> (data[pos + 2]) * base::pl_list_a);
                    set_color(state == 0, pl_list_info, 1, data[pos + 1]);
                    std::cout << "  -------------------------------  ";
                }
                std::cout << color::defaul_color;
                pos += msg_sizes::pl_list;
                break;

            case volume_info:
                if (pos + msg_sizes::volume > data.size())
                    goto end_for;
                set_cursor(base::volume_0);
                set_color(state == 2, volume_info, 0);
                std::cout << data.substr(pos + 1, 5/*volume_width*/);
                set_cursor(base::volume_1);
                set_color(state == 2, volume_info, 1);
                std::cout << data.substr(pos + 1 + volume_width, 5/*volume_width*/);
                std::cout << color::defaul_color;
                pos += msg_sizes::volume;
                break;

            case state_info:
                if (pos + 2 > data.size())
                    goto end_for;
                state = static_cast <size_t> (data[pos + 1]);
                pos += 2;
                break;

            default:
                pos++;
                break;
        }
    }
    end_for:
    if (pos != 0)
        data = data.substr(pos);
    std::cout.flush();
}


