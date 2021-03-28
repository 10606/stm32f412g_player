#include <iostream>
#include "interactive/common_types.h"

struct displayed_song_info_t
{
    uint8_t cmd;
    char selected;
    uint8_t pos;
    char line_0[11];
    char line_1[21];
};

int main ()
{
    
    displayed_song_info_t song_info;

    song_info.pos = 123;
    song_info.selected = 'a';
    
    std::cout << has_pos <displayed_song_info_t> :: value << '\n';
    std::cout << has_selected <displayed_song_info_t> :: value << '\n';
    
    std::cout << get_pos(song_info) << '\n';
    std::cout << get_selected(song_info) << '\n';
}

