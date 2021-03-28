#include "term_colors.h"

const std::string color::defaul_color = "\033[00;00m"; 

const std::string color::white = "\033[00;37m"; 
const std::string color::red = "\033[00;31m"; 
const std::string color::cyan = "\033[00;36m";
const std::string color::green = "\033[00;32m";
const std::string color::yellow = "\033[00;33m";


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
            {{color::green}}
        },
        { //song
            {{color::green}}
        }
    },
    
    { //displayed_song_info
        { //group
            { //not selected
                {
                    color::white, //0
                    color::cyan, //selected
                    color::red, //played
                    color::cyan //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::green, //selected
                    color::yellow, //played
                    color::green //selected and played
                }
            }
        },
        { //song
            { //not selected
                {
                    color::white, //0
                    color::cyan, //selected
                    color::red, //played
                    color::red //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::green, //selected
                    color::yellow, //played
                    color::yellow //selected and played
                }
            }
        }
    },
    
    { //pl_list_info
        { //first line
            { //not selected
                {
                    color::white, //0
                    color::cyan, //selected
                    color::red, //played
                    color::cyan //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::green, //selected
                    color::yellow, //played
                    color::green //selected and played
                }
            }
        },
        { //second line
            { //not selected
                {
                    color::white, //0
                    color::cyan, //selected
                    color::red, //played
                    color::red //selected and played
                }
            },
            { //selected
                {
                    color::cyan, //0
                    color::green, //selected
                    color::yellow, //played
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
