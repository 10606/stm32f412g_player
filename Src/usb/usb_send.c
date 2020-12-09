#include "usb_send.h"

#include "display.h"

char cur_song_info = 0x01;
char displayed_song_info = 0x02;
char pl_list_info = 0x03;
char volume_info = 0x04;
char state_info = 0x05;

void send_cur_song 
(
    char cur_song_name[song_name_sz + 1], 
    char cur_group_name[group_name_sz + 1]
)
{
    char buffer[song_name_sz + 1 + group_name_sz + 1 + 1];
    buffer[0] = cur_song_info;
    memcpy(buffer + 1, cur_song_name, song_name_sz + 1);
    memcpy(buffer + 1 + song_name_sz + 1, cur_group_name, group_name_sz + 1);
    CDC_Transmit_FS(buffer, sizeof(buffer));
}

void send_displayed_song 
(
    char s_group[name_offset + group_name_sz + 1], 
    char s_song[name_offset + song_name_sz + 1], 
    char selected, 
    uint32_t pos
)
{
    char buffer[name_offset + group_name_sz + 1 + name_offset + song_name_sz + 1 + 3];
    buffer[0] = displayed_song_info;
    buffer[1] = selected;
    buffer[2] = pos;
    memcpy(buffer + 3, s_group, name_offset + group_name_sz + 1);
    memcpy(buffer + 3 + name_offset + group_name_sz + 1, s_song, name_offset + song_name_sz + 1);
    CDC_Transmit_FS(buffer, sizeof(buffer));
}

void send_pl_list
(
    char s_playlist[name_offset + pl_name_sz + count_offset + 4],
    char selected, 
    uint32_t pos
)
{
    char buffer[name_offset + pl_name_sz + count_offset + 4 + 3];
    buffer[0] = pl_list_info;
    buffer[1] = selected;
    buffer[2] = pos;
    memcpy(buffer + 3, s_playlist, name_offset + pl_name_sz + count_offset + 4);
    CDC_Transmit_FS(buffer, sizeof(buffer));
}

void send_volume
(
    char s_volume[volume_width],
    char s_state[volume_width]
)
{
    char buffer[volume_width + volume_width + 1];
    buffer[0] = volume_info;
    memcpy(buffer + 1, s_volume, volume_width);
    memcpy(buffer + 1 + volume_width, s_state, volume_width);
    CDC_Transmit_FS(buffer, sizeof(buffer));
}

void send_state
(
    int state
)
{
    char buffer[2];
    buffer[0] = state_info;
    buffer[1] = state & 0xff;
    CDC_Transmit_FS(buffer, sizeof(buffer));
}

