#include "headers.h"

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
    
    static const constexpr coord cur_song_0{5, 1};
    static const constexpr coord cur_song_1{5, 2};

    static const constexpr coord disp_song_0{name_offset + pl_name_sz + count_offset + 4 + 2, 4};
    static const constexpr coord disp_song_1{name_offset + pl_name_sz + count_offset + 4 + 2, 5};
    static const constexpr coord disp_song_a{0, 2};

    static const constexpr coord pl_list_0{0, 4};
    static const constexpr coord pl_list_a{0, 2};
    
    static const constexpr coord volume_0{name_offset + pl_name_sz + count_offset + 4 + 2 + 5, 1};
    static const constexpr coord volume_1{name_offset + pl_name_sz + count_offset + 4 + 2 + 5, 2};
};


struct color
{
    static const std::string defaul_color; 

    static const std::string white; 
    static const std::string red; 
    static const std::string cyan;
    static const std::string green;
    static const std::string yellow;
};

const std::string color::defaul_color = "\033[00;00m"; 

const std::string color::white = "\033[00;37m"; 
const std::string color::red = "\033[00;31m"; 
const std::string color::cyan = "\033[00;36m";
const std::string color::green = "\033[00;32m";
const std::string color::yellow = "\033[00;33m";


struct colors
{
    static const std::vector  //cmd
    <
        std::vector //line
        <
            std::vector //current
            <
                std::vector //s
                <
                    std::string
                >
            >
        >
    > table;
};


const std::vector  //cmd
<
    std::vector //line
    <
        std::vector //current
        <
            std::vector //s
            <
                std::string
            >
        >
    >
> colors::table = 
{
    {},
    
    { //cur_song_info
        { //group
            {{color::yellow}}
        },
        { //song
            {{color::yellow}}
        }
    },
    
    { //displayed_song_info
        { //group
            { //not selected
                {
                    color::white, //0
                    color::yellow, //selected
                    color::green, //played
                    color::yellow //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::yellow, //selected
                    color::green, //played
                    color::yellow //selected and played
                }
            }
        },
        { //song
            { //not selected
                {
                    color::white, //0
                    color::yellow, //selected
                    color::green, //played
                    color::green //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::yellow, //selected
                    color::green, //played
                    color::green //selected and played
                }
            }
        }
    },
    
    { //pl_list_info
        {
            { //not selected
                {
                    color::white, //0
                    color::yellow, //selected
                    color::green, //played
                    color::yellow //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::yellow, //selected
                    color::green, //played
                    color::yellow //selected and played
                }
            }
        }
    },
    
    { //volume_info
        { //volume
            { //not selected
                {color::white}
            },
            { //selected
                {color::cyan}
            }
        },
        { //state
            { //not selected
                {color::white}
            },
            { //selected
                {color::cyan}
            }
        }
    }
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
        line = 0;
    }
    if (cmd == volume_info)
    {
        s = 0;
    }
    std::cout << colors::table[cmd][line][current][s];
}

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
                if (pos + song_name_sz + 1 + group_name_sz + 1 + 1 > data.size())
                    goto end_for;
                set_cursor(base::cur_song_0);
                set_color(0, cur_song_info, 0);
                std::cout << data.substr(pos + 1 + song_name_sz + 1, group_name_sz + 1);
                set_cursor(base::cur_song_1);
                set_color(0, cur_song_info, 1);
                std::cout << data.substr(pos + 1, song_name_sz + 1);
                std::cout << color::defaul_color;
                pos += song_name_sz + 1 + group_name_sz + 1 + 1;
                break;

            case displayed_song_info:
                if (pos + name_offset + group_name_sz + 1 + name_offset + song_name_sz + 1 + 3 > data.size())
                    goto end_for;
                //std::cout << ((data[pos + 1] & 1)? 's' : ' ') << " " << static_cast <size_t> (data[pos + 2]) << "\n";
                set_cursor(base::disp_song_0 + static_cast <size_t> (data[pos + 2]) * base::disp_song_a);
                set_color(state == 1, displayed_song_info, 0, data[pos + 1]);
                std::cout << data.substr(pos + 3, name_offset + group_name_sz + 1);
                set_cursor(base::disp_song_1 + static_cast <size_t> (data[pos + 2]) * base::disp_song_a);
                set_color(state == 1, displayed_song_info, 1, data[pos + 1]);
                std::cout << data.substr(pos + 3 + name_offset + group_name_sz + 1, name_offset + song_name_sz + 1);
                std::cout << color::defaul_color;
                pos += name_offset + group_name_sz + 1 + name_offset + song_name_sz + 1 + 3;
                break;

            case pl_list_info:
                if (pos + name_offset + pl_name_sz + count_offset + 4 + 3 > data.size())
                    goto end_for;
                //std::cout << (data[pos + 1]? 's' : ' ') << " " << data[pos + 2] << "\n";
                set_cursor(base::pl_list_0 + static_cast <size_t> (data[pos + 2]) * base::pl_list_a);
                set_color(state == 0, pl_list_info, 0, data[pos + 1]);
                std::cout << data.substr(pos + 3, name_offset + pl_name_sz + count_offset + 4);
                std::cout << color::defaul_color;
                pos += name_offset + pl_name_sz + count_offset + 4 + 3;
                break;

            case volume_info:
                if (pos + volume_width + volume_width + 1 > data.size())
                    goto end_for;
                set_cursor(base::volume_0);
                set_color(state == 2, volume_info, 0);
                std::cout << data.substr(pos + 1, 5/*volume_width*/);
                set_cursor(base::volume_1);
                set_color(state == 2, volume_info, 1);
                std::cout << data.substr(pos + 1 + volume_width, 5/*volume_width*/);
                std::cout << color::defaul_color;
                pos += volume_width + volume_width + 1;
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


