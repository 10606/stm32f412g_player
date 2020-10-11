#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>

#define command_up 0x01
#define command_down 0x02

//left
#define command_back 0x03
//center
#define command_forward 0x04
//right
#define command_select 0x05

#define commnad_play_pause 0x06
#define command_repeat 0x09

#define commnad_volume_up 0x07
#define command_volume_down 0x08

#define command_seek_forward 0x0a
#define command_seek_backward 0x0b

#define command_next_song 0x0c
#define commnad_prev_song 0x0d

uint32_t usb_process ();

#endif

