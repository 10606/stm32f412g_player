#ifndef AUDIO_H
#define AUDIO_H

#include "FAT.h"
#include "id3.h"
#include <stdint.h>
#include <stddef.h>

size_t const mp3_frames_in_buffer = 4;
size_t const decoded_mp3_frame_size = 4608;
size_t const audio_buffer_size = (mp3_frames_in_buffer * decoded_mp3_frame_size);
uint8_t const audio_default_volume = 70;

enum buffer_state_t
{
    buffer_offset_none = 0,  
    buffer_offset_half,  
    buffer_offset_full,     
};

struct audio_ctl_t 
{
    file_descriptor audio_file;
    uint8_t buff[audio_buffer_size];
    buffer_state_t state;
    mp3_info info;
    uint32_t audio_freq;
    uint8_t volume;
    bool pause_status;
    bool repeat_mode;
    bool seeked;
};
extern audio_ctl_t  audio_ctl;

void audio_init ();
void audio_destruct ();
uint32_t audio_start ();
uint32_t audio_process (uint8_t * need_redraw);

#endif

