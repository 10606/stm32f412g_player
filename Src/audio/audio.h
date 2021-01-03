#ifndef AUDIO_H
#define AUDIO_H

#include "FAT.h"
#include "id3.h"
#include <stdint.h>

//#define AUDIO_BUFFER_SIZE       8192  * 2
//#define AUDIO_BUFFER_SIZE       12288 
#define MP3_FRAMES_IN_BUFFER    4
#define MP3_FRAME_SIZE          4608
#define AUDIO_BUFFER_SIZE       (MP3_FRAMES_IN_BUFFER * MP3_FRAME_SIZE)
#define AUDIO_DEFAULT_VOLUME    70
#define HEADBAND_HEIGHT         72

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

typedef struct audio_ctl {
    file_descriptor audio_file;
    uint8_t buff[AUDIO_BUFFER_SIZE];
    buffer_state_t state;
    audio_playback_state_t audio_state;
    uint32_t volume;
    uint32_t pause_status;
    uint32_t audio_freq;
    char repeat_mode;
    char seeked;
    mp3_info info;
} audio_ctl;
extern audio_ctl  buffer_ctl;


void audio_init ();
void audio_play ();
void audio_destruct ();
uint8_t audio_process ();

#endif

