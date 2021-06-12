#ifndef AUDIO_H
#define AUDIO_H

#include "FAT.h"
#include "id3.h"
#include <stdint.h>
#include <stddef.h>

enum buffer_state_t
{
    buffer_offset_none = 0,  
    buffer_offset_half,  
    buffer_offset_full,     
};

struct audio_ctl_t 
{
    audio_ctl_t ();
    uint32_t audio_init ();
    void audio_destruct ();
    uint32_t audio_process ();
    
    static size_t const mp3_frames_in_buffer = 4;
    static size_t const decoded_mp3_frame_size = 4608;
    static size_t const audio_buffer_size = (mp3_frames_in_buffer * decoded_mp3_frame_size);

    uint32_t audio_freq;
    uint8_t volume;
    bool pause_status;
    bool repeat_mode;
    bool seeked;
    buffer_state_t state;
    mp3_info info;
    file_descriptor audio_file;
    uint8_t buff[audio_buffer_size];
    bool need_redraw;
    
private:
    
    struct tik_t
    {
        uint16_t min;
        uint16_t sec;
        uint16_t ms;
    };

    void byte_to_time (tik_t * time, uint32_t value) const;
    void next_pcm_part ();
    void display_time () const;
    uint32_t new_song_or_repeat ();
};

extern audio_ctl_t audio_ctl;

#endif

