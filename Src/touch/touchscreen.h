#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "view.h"
#include <stdint.h>

typedef enum direction_t
{
    NON_DIRECTION   = 0,
    LEFT_DIRECTION  = 1,
    RIGHT_DIRECTION = 2,
    UP_DIRECTION    = 3,
    DOWN_DIRECTION  = 4
} direction_t;

typedef struct old_touch_state
{
    int32_t start_x, start_y;
    int32_t old_x, old_y;
    int32_t dolg_x, dolg_y;
    uint32_t direction_mask;
    uint32_t time; // need to calc speed
    uint8_t pressed; // 1 if press
    uint8_t moved; // 1 if |old_x - new_x| > 20 while pressed
} old_touch_state;

void touch_check (old_touch_state * ots, view * vv, uint8_t * need_redraw);

#endif

