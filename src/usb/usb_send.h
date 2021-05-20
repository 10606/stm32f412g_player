#ifndef USB_SEND_H
#define USB_SEND_H

#include "usb_commands.h"
#include "usbd_cdc_if.h"
#include "view.h"
#include <stdint.h>
#include <type_traits>

uint8_t send_cur_song 
(
    std::decay_t <decltype(cur_song_info_t::line_0)> cur_group_name,
    std::decay_t <decltype(cur_song_info_t::line_1)> cur_song_name
);

uint8_t send_displayed_song 
(
    std::decay_t <decltype(displayed_song_info_t::line_0)> s_group,
    std::decay_t <decltype(displayed_song_info_t::line_1)> s_song,
    char selected, 
    uint32_t pos
);

uint8_t send_pl_list
(
    std::decay_t <decltype(pl_list_info_t::name)> s_playlist,
    char selected, 
    uint32_t pos
);

uint8_t send_volume
(
    std::decay_t <decltype(volume_info_t::line_0)> s_volume,
    std::decay_t <decltype(volume_info_t::line_1)> s_state
);

uint8_t send_state
(
    state_t state
);

uint8_t send_empty ();

#endif

