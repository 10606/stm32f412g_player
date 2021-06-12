#ifndef USB_COMMANDS_H
#define USB_COMMANDS_H

#include <stdint.h>
#include <stddef.h>
#include "char_sizes.h"
#include "view_states.h"

// WARNING on change: usb_process (view * vv)
typedef enum
{
    command_nop = 0x00,
    command_up = 0x01,
    command_down = 0x02,

    command_back = 0x03,    //left
    command_forward = 0x04, //center
    command_select = 0x05,  //right

    commnad_play_pause = 0x06,

    commnad_volume_up = 0x07,
    command_volume_down = 0x08,

    command_repeat = 0x09,

    command_seek_forward = 0x0a,
    command_seek_backward = 0x0b,

    command_next_song = 0x0c,
    commnad_prev_song = 0x0d,

    commnad_send_info = 0x0e
} usb_command;

typedef enum
{
    cur_song_info = 0x01,
    displayed_song_info = 0x02,
    pl_list_info = 0x03,
    volume_info = 0x04,
    state_info = 0x05
} sended_info;

struct cur_song_info_t
{
    uint8_t cmd;
    char line_0[sz::group_name + 1];
    char line_1[sz::song_name + 1];
};

struct displayed_song_info_t
{
    uint8_t cmd;
    char selected;
    uint8_t pos;
    char line_0[sz::number + sz::group_name + 1];
    char line_1[sz::number + sz::song_name + 1];
};

struct pl_list_info_t
{
    uint8_t cmd;
    char selected;
    uint8_t pos;
    char name[sz::count_offset + sz::count + 1];
};

struct volume_info_t
{
    uint8_t cmd;
    char line_0[sz::volume];
    char line_1[sz::volume];
};

struct state_info_t
{
    uint8_t cmd;
    state_t state;
};

#endif

