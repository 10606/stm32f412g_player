#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>
#include "view.h"

enum class direction_t
{
    non   = 0,
    left  = 1,
    right = 2,
    up    = 3,
    down  = 4
};

struct point
{
    int32_t x, y;
};

struct touch_processing
{
    uint32_t touch_region (view * vv, bool & need_redraw);
    uint32_t move_left  (int32_t & ans, int32_t offset, char speed, view * vv, bool & need_redraw);
    uint32_t move_right (int32_t & ans, int32_t offset, char speed, view * vv, bool & need_redraw);
    uint32_t move_up    (int32_t & ans, int32_t offset, char speed, view * vv, bool & need_redraw);
    uint32_t move_down  (int32_t & ans, int32_t offset, char speed, view * vv, bool & need_redraw);

    uint32_t do_move 
    (
        int32_t & ans,
        direction_t direction,
        int32_t offset, 
        char speed, 
        view * vv, 
        bool & need_redraw
    );
    
    point start;
    uint32_t direction_mask;
    
private:
    uint32_t move_left_right
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv, 
        bool & need_redraw,
        uint8_t direction // 0 left, 1 - right
    );
    
    uint32_t move_up_down
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv, 
        bool & need_redraw,
        uint8_t direction // 0 - down, 1 - up 
    );
};

#endif

