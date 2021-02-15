#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>
#include "view.h"
#include "touchscreen.h"

void touch_region 
(
    old_touch_state * ots,
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_left  
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_right 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_up    
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_down  
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
);

extern int32_t (* const do_move[5]) 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
);

#endif

