#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "view.h"
#include "moving.h"
#include <stdint.h>

struct touch_state
{
    touch_state ();
    ret_code touch_check (view * vv);
    void on_timer ();

    touch_processing state;
    point old;
    point dolg;
    volatile uint32_t tick_counter;
    bool press; // 1 if press
    bool moved; // 1 if |old_x - new_x| > 20 while pressed

private:
    
    bool is_moved (point p) const;
    direction_n::type get_direction (point p) const;
    ret_code unpressed (view * vv);
    ret_code pressed (point p, view * vv);
};

extern touch_state touch;

#endif

