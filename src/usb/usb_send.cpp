#include "usb_send.h"

#include "view.h"
#include "display.h"
#include "usb_commands.h"

uint8_t send_cur_song 
(
    char cur_group_name[group_name_sz + 1],
    char cur_song_name[song_name_sz + 1]
)
{
    cur_song_info_t answer;
    answer.cmd = cur_song_info;
    memcpy(answer.line_0, cur_group_name, group_name_sz + 1);
    memcpy(answer.line_1, cur_song_name, song_name_sz + 1);
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_displayed_song 
(
    char s_group[name_offset + group_name_sz + 1], 
    char s_song[name_offset + song_name_sz + 1], 
    char selected, 
    uint32_t pos
)
{
    displayed_song_info_t answer;
    answer.cmd = displayed_song_info;
    answer.selected = selected;
    answer.pos = pos;
    memcpy(answer.line_0, s_group, name_offset + group_name_sz + 1);
    memcpy(answer.line_1, s_song, name_offset + song_name_sz + 1);
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_pl_list
(
    char s_playlist[sizeof(pl_list_info_t::name)],
    char selected, 
    uint32_t pos
)
{
    pl_list_info_t answer;
    answer.cmd = pl_list_info;
    answer.selected = selected;
    answer.pos = pos;
    memcpy(answer.name, s_playlist, sizeof(answer.name));
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_volume
(
    char s_volume[volume_width],
    char s_state[volume_width]
)
{
    volume_info_t answer;
    answer.cmd = volume_info;
    memcpy(answer.line_0, s_volume, volume_width);
    memcpy(answer.line_1, s_state, volume_width);
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_state
(
    state_t state
)
{
    state_info_t answer;
    answer.cmd = state_info;
    answer.state = state;
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

