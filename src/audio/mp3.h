#ifndef AUDIO_MP3_H
#define AUDIO_MP3_H

#include <stdint.h>

#include "mad.h"
#include "FAT.h"

#define mp3_input_buffer_size 1536LU * 2
#define mp3_frame_size 1152LU

typedef struct 
{
    uint8_t buffer[mp3_input_buffer_size + MAD_BUFFER_GUARD];
    uint32_t size;
} mp3_input_buffer_t;

typedef struct
{
    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;
} mad_data_t;

void init_mad ();
void deinit_mad ();
void reuse_mad ();
uint32_t get_pcm_sound (file_descriptor * _file, uint8_t * buffer, uint32_t size);

#endif

