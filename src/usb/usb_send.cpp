#include "usb_send.h"

#include "view.h"
#include "usb_commands.h"

sender_t sender;

uint8_t sender_t::flush ()
{
    if (pos == 0)
        return 0;
    uint8_t ret = CDC_Transmit_FS(buffer, pos);
    if (ret == 0)
        pos = 0;
    return ret;
}

uint8_t sender_t::send_cur_song 
(
    std::decay_t <decltype(cur_song_info_t::line_0)> cur_group_name,
    std::decay_t <decltype(cur_song_info_t::line_1)> cur_song_name
)
{
    cur_song_info_t answer;
    answer.cmd = cur_song_info;
    memcpy(answer.line_0, cur_group_name, sizeof(answer.line_0));
    memcpy(answer.line_1, cur_song_name, sizeof(answer.line_1));
    return add_to_buff((uint8_t *)&answer, sizeof(answer));
}

uint8_t sender_t::send_displayed_song 
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
    return add_to_buff((uint8_t *)&answer, sizeof(answer));
}

uint8_t sender_t::send_pl_list
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
    return add_to_buff((uint8_t *)&answer, sizeof(answer));
}

uint8_t sender_t::send_volume
(
    std::decay_t <decltype(volume_info_t::line_0)> s_volume,
    std::decay_t <decltype(volume_info_t::line_1)> s_state
)
{
    volume_info_t answer;
    answer.cmd = volume_info;
    memcpy(answer.line_0, s_volume, sizeof(answer.line_0));
    memcpy(answer.line_1, s_state, sizeof(answer.line_1));
    return add_to_buff((uint8_t *)&answer, sizeof(answer));
}

uint8_t sender_t::send_state
(
    state_t state
)
{
    state_info_t answer;
    answer.cmd = state_info;
    answer.state = state;
    return add_to_buff((uint8_t *)&answer, sizeof(answer));
}

uint8_t sender_t::send_empty ()
{
    uint8_t ret = USBD_OK;
    {  
        decltype(volume_info_t::line_0) s_volume;
        decltype(volume_info_t::line_1) s_state;
        memset(s_volume, ' ', sizeof(s_volume));
        memset(s_state, ' ', sizeof(s_state));
        HAL_Delay(1);
        ret |= send_volume(s_volume, s_state);
    }
    {
        decltype(cur_song_info_t::line_0) cur_group_name;
        decltype(cur_song_info_t::line_1) cur_song_name;
        memset(cur_group_name, ' ', sizeof(cur_group_name));
        memset(cur_song_name, ' ', sizeof(cur_song_name));
        HAL_Delay(1);
        ret |= send_cur_song(cur_group_name, cur_song_name);
    }
    {
        char selected[playlist_view::view_cnt];
        memset(selected, 0, sizeof(selected));
        
        decltype(displayed_song_info_t::line_0) s_group;
        decltype(displayed_song_info_t::line_1) s_song;
        memset(s_group, ' ', sizeof(s_group));
        memset(s_song, ' ', sizeof(s_song));
    
        for (uint32_t i = 0; i != playlist_view::view_cnt; ++i)
        {
            HAL_Delay(1);
            ret |= send_displayed_song(s_group, s_song, selected[i], i);
        }
    }
    {
        char selected[pl_list::view_cnt];
        memset(selected, 0, sizeof(selected));
        decltype(pl_list_info_t::name) s_playlist;
        memset(s_playlist, ' ', sizeof(s_playlist));
        
        for (uint32_t i = 0; i != pl_list::view_cnt; ++i)
        {
            HAL_Delay(1);
            ret |= send_pl_list(s_playlist, selected[i], i);
        }
    }
    return ret;
}

uint8_t sender_t::add_to_buff (uint8_t * value, uint32_t size)
{
    if (size >= std::extent_v <decltype(buffer)>)
        return USBD_BUSY;
    
    if (size + pos >= std::extent_v <decltype(buffer)>)
    {
        uint8_t ret = CDC_Transmit_FS(buffer, pos);
        pos = size;
        memcpy(buffer, value, size);
        return ret;
    }
    else
    {
        uint8_t ret = 0;
        memcpy(buffer + pos, value, size);
        pos += size;
        if (2 * pos >= std::extent_v <decltype(buffer)>)
        {
            ret = CDC_Transmit_FS(buffer, pos);
            if (ret == 0)
                pos = 0;
        }
        return ret;
    }
}

