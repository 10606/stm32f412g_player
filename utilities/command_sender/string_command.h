#ifndef STRING_COMMAND_H
#define STRING_COMMAND_H

const char * const str_command_up = "up";
const char * const str_command_down = "down";

const char * const str_command_back = "back";
const char * const str_command_forward = "forward";
const char * const str_command_select = "select";

const char * const str_command_play_pause = "play_pause";
const char * const str_command_repeat = "repeat";

const char * const str_command_volume_up = "volume_up";
const char * const str_command_volume_down = "volume_down";

const char * const str_command_seek_forward = "seek_forward";
const char * const str_command_seek_backward = "seek_backward";

const char * const str_command_next_song = "next_song";
const char * const str_command_prev_song = "prev_song";

const char * const str_commands[] =
{
    "",                     //0x00

    str_command_up,         //0x01
    str_command_down,       //0x02

    str_command_back,       //0x03
    str_command_forward,    //0x04
    str_command_select,     //0x05

    str_command_play_pause, //0x06

    str_command_volume_up,  //0x07
    str_command_volume_down,//0x08

    str_command_repeat,     //0x09

    str_command_seek_forward,  //0x0a
    str_command_seek_backward, //0x0b

    str_command_next_song,  //0x0c
    str_command_prev_song   //0x0d
};

#endif

