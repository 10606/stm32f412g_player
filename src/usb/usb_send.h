#ifndef USB_SEND_H
#define USB_SEND_H

#include "usbd_cdc_if.h"
#include "display.h"
#include "view.h"
#include <stdint.h>

uint8_t send_cur_song 
(
    char cur_group_name[group_name_sz + 1],
    char cur_song_name[song_name_sz + 1]
);

uint8_t send_displayed_song 
(
    char s_group[name_offset + group_name_sz + 1], 
    char s_song[name_offset + song_name_sz + 1], 
    char selected, 
    uint32_t pos
);

uint8_t send_pl_list
(
    char s_playlist[name_offset + pl_name_sz + count_offset + 4],
    char selected, 
    uint32_t pos
);

uint8_t send_volume
(
    char s_volume[volume_width],
    char s_state[volume_width]
);

uint8_t send_state
(
    state_t state
);

#endif

