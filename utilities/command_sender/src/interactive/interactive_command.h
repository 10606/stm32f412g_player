#ifndef INTERACTIVE_COMMAND_H
#define INTERACTIVE_COMMAND_H

#include <array>
#include <string_view>

const char * const int_command_up = "\033[A";
const char * const int_command_down = "\033[B";

const char * const int_command_back = "\033[D";
const char * const int_command_select = "\n";
const char * const int_command_forward = "\033[C";

const char * const int_command_play_pause = " ";
const char * const int_command_end_pause = "p";
const char * const int_command_repeat = "L";

const char * const int_command_volume_up = "\033[24~";
const char * const int_command_volume_down = "\033[23~";

const char * const int_command_seek_forward = "f";
const char * const int_command_seek_backward = "b";

const char * const int_command_next_song = ">";
const char * const int_command_prev_song = "<";

const char * const int_command_send_info = "R";

const char * const int_command_find_next = "n";
const char * const int_command_find_set = "/";

const char * const int_command_set_song = "s";

const char * const int_command_jmp = "gg";

const char * const quit = "q";

std::array <std::string_view, 21> const int_commands =
{
    "",                     //0x00

    int_command_up,         //0x01
    int_command_down,       //0x02

    int_command_back,       //0x03
    int_command_forward,    //0x04
    int_command_select,     //0x05

    int_command_play_pause, //0x06

    int_command_volume_up,  //0x07
    int_command_volume_down,//0x08

    int_command_repeat,     //0x09

    int_command_seek_forward,  //0x0a
    int_command_seek_backward, //0x0b

    int_command_next_song,  //0x0c
    int_command_prev_song,  //0x0d

    int_command_send_info,  //0x0e
    
    int_command_end_pause,  //0x0f
    
    int_command_find_next,  //0x10

    int_command_set_song, //0x11

    // ---------------------------
    
    int_command_find_set,   //0x12
    
    int_command_jmp,        //0x13
    
    quit                    //0x14
};

#endif

