#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "audio.h"

inline uint32_t min (uint32_t a, uint32_t b)
{
    return (a < b)? a : b;
}

inline uint32_t max (uint32_t a, uint32_t b)
{
    return (a > b)? a : b;
}


typedef struct tik_t
{
    uint16_t min;
    uint16_t sec;
    uint16_t ms;
} tik_t;

static inline void byte_to_time (tik_t * time, uint32_t value)
{
    if (value >= buffer_ctl.info.offset)
        value -= buffer_ctl.info.offset;
    else
        value = 0;

    uint32_t time_ms = 
        (float)(buffer_ctl.info.length) /
        (float)(buffer_ctl.audio_file.size - buffer_ctl.info.offset) *
        (float)(value);
    
    time->ms = time_ms % 1000;
    time->sec = (time_ms / 1000) % 60;
    time->min = time_ms / 1000 / 60;
}


#endif

