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
    int32_t x,y;
};

struct touch_processing
{
    void touch_region (view * vv, uint8_t * need_redraw);
    int32_t move_left (int32_t offset, char speed, view * vv, uint8_t * need_redraw);
    int32_t move_right (int32_t offset, char speed, view * vv, uint8_t * need_redraw);
    int32_t move_up (int32_t offset, char speed, view * vv, uint8_t * need_redraw);
    int32_t move_down (int32_t offset, char speed, view * vv, uint8_t * need_redraw);

    int32_t do_move 
    (
        direction_t direction,
        int32_t offset, 
        char speed, 
        view * vv, 
        uint8_t * need_redraw
    );
    
    point start;
    uint32_t direction_mask;
    
private:
    int32_t move_left_right
    (
        int32_t offset, 
        char speed, 
        view * vv, 
        uint8_t * need_redraw,
        uint8_t direction // 0 left, 1 - right
    );
    int32_t move_up_down
    (
        int32_t offset, 
        char speed, 
        view * vv, 
        uint8_t * need_redraw,
        uint8_t direction // 0 - down, 1 - up 
    );
};

#endif

