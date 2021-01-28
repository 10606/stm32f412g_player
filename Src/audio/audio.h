#ifndef AUDIO_H
#define AUDIO_H

#include "FAT.h"
#include "id3.h"
#include <stdint.h>

#define mp3_frames_in_buffer    4
#define decoded_mp3_frame_size  4608
#define audio_buffer_size       (mp3_frames_in_buffer * decoded_mp3_frame_size)
#define audio_default_volume    70

typedef enum {
    AUDIO_ERROR_NONE = 0,
    AUDIO_ERROR_NOTREADY,
    AUDIO_ERROR_IO,
    AUDIO_ERROR_EOF,
} audio_error_t;

typedef enum {
    AUDIO_STATE_IDLE = 0,
    AUDIO_STATE_INIT,    
    AUDIO_STATE_PLAYING,  
} audio_playback_state_t;

typedef enum {
    BUFFER_OFFSET_NONE = 0,  
    BUFFER_OFFSET_HALF,  
    BUFFER_OFFSET_FULL,     
} buffer_state_t;

typedef struct audio_ctl_t {
    file_descriptor audio_file;
    uint8_t buff[audio_buffer_size];
    buffer_state_t state;
    audio_playback_state_t audio_state;
    uint32_t volume;
    uint32_t pause_status;
    uint32_t audio_freq;
    char repeat_mode;
    char seeked;
    mp3_info info;
} audio_ctl_t;
extern audio_ctl_t  audio_ctl;


void audio_init ();
void audio_play ();
void audio_destruct ();
uint32_t audio_process ();

#endif

