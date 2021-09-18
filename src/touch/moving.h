#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>
#include "view.h"

namespace direction_n = directions::nlrud;

struct point
{
    int32_t x, y;
};

struct touch_processing
{
    uint32_t touch_region (view * vv);
    uint32_t move_left  (int32_t & ans, int32_t offset, char speed, view * vv);
    uint32_t move_right (int32_t & ans, int32_t offset, char speed, view * vv);
    uint32_t move_up    (int32_t & ans, int32_t offset, char speed, view * vv);
    uint32_t move_down  (int32_t & ans, int32_t offset, char speed, view * vv);

    uint32_t do_move 
    (
        int32_t & ans,
        direction_n::type direction,
        int32_t offset, 
        char speed, 
        view * vv
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
        directions::lr::type direction
    );
    
    uint32_t move_up_down
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv, 
        directions::du::type direction
    );
};

#endif

