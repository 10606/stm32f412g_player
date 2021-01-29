#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>
#include "view.h"

void touch_region 
(
    int32_t x, 
    int32_t y, 
    uint32_t * direction_mask, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_left  
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    uint32_t * direction_mask, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_right 
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    uint32_t * direction_mask, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_up    
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    uint32_t * direction_mask, 
    view * vv, 
    uint8_t * need_redraw
);

int32_t move_down  
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    uint32_t * direction_mask, 
    view * vv, 
    uint8_t * need_redraw
);

#endif

