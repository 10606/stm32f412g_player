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
    ret_code touch_region (view * vv);
    ret_code move_left  (int32_t & ans, int32_t offset, char speed, view * vv);
    ret_code move_right (int32_t & ans, int32_t offset, char speed, view * vv);
    ret_code move_up    (int32_t & ans, int32_t offset, char speed, view * vv);
    ret_code move_down  (int32_t & ans, int32_t offset, char speed, view * vv);

    ret_code do_move 
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
    ret_code move_left_right
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv,
        directions::lr::type direction
    );
    
    ret_code move_up_down
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv, 
        directions::du::type direction
    );
};

#endif

