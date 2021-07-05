#ifndef USB_COMMANDS_H
#define USB_COMMANDS_H

#include <stdint.h>
#include <stddef.h>
#include "char_sizes.h"
#include "view_states.h"

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

