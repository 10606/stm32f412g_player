#include "usb_send.h"

#include "view.h"
#include "usb_commands.h"

uint8_t send_cur_song 
(
    std::decay_t <decltype(cur_song_info_t::line_0)> cur_group_name,
    std::decay_t <decltype(cur_song_info_t::line_1)> cur_song_name
)
{
    cur_song_info_t answer;
    answer.cmd = cur_song_info;
    memcpy(answer.line_0, cur_group_name, sizeof(answer.line_0));
    memcpy(answer.line_1, cur_song_name, sizeof(answer.line_1));
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_displayed_song 
(
    std::decay_t <decltype(displayed_song_info_t::line_0)> s_group,
    std::decay_t <decltype(displayed_song_info_t::line_1)> s_song,
    char selected, 
    uint32_t pos
)
{
    displayed_song_info_t answer;
    answer.cmd = displayed_song_info;
    answer.selected = selected;
    answer.pos = pos;
    memcpy(answer.line_0, s_group, sizeof(answer.line_0));
    memcpy(answer.line_1, s_song, sizeof(answer.line_1));
    return CDC_Transmit_FS((uint8_t *)&answer, sizeof(answer));
}

uint8_t send_pl_list
(
    std::decay_t <decltype(pl_list_info_t::name)> s_playlist,
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
    std::decay_t <decltype(volume_info_t::line_0)> s_volume,
    std::decay_t <decltype(volume_info_t::line_1)> s_state
)
{
    volume_info_t answer;
    answer.cmd = volume_info;
    memcpy(answer.line_0, s_volume, sizeof(answer.line_0));
    memcpy(answer.line_1, s_state, sizeof(answer.line_1));
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

