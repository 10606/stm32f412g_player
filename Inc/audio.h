#ifndef AUDIO_H
#define AUDIO_H

#include "FAT.h"
#include <stdint.h>

//#define AUDIO_BUFFER_SIZE       8192  * 2
#define AUDIO_BUFFER_SIZE       12288 
#define AUDIO_DEFAULT_VOLUME    70
#define HEADBAND_HEIGHT         72

typedef enum {
    AUDIO_STATE_IDLE = 0,
    AUDIO_STATE_INIT,    
    AUDIO_STATE_PLAYING,  
} AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
    BUFFER_OFFSET_NONE = 0,  
    BUFFER_OFFSET_HALF,  
    BUFFER_OFFSET_FULL,     
} BUFFER_StateTypeDef;

typedef struct audio_ctl {
    file_descriptor audio_file;
    uint8_t buff[AUDIO_BUFFER_SIZE];
    uint32_t fptr;  
    uint32_t audio_file_size; //TODO remove
    BUFFER_StateTypeDef state;
    AUDIO_PLAYBACK_StateTypeDef audio_state;
    uint32_t volume;
    uint32_t pause_status;
    uint32_t *audio_freq_ptr;
} audio_ctl;

extern audio_ctl  buffer_ctl;

#endif

