#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "view.h"
#include "moving.h"
#include <stdint.h>

struct touch_state
{
    touch_state ();
    void touch_check (view * vv, bool & need_redraw);

    touch_processing state;
    point old;
    point dolg;
    uint32_t time; // need to calc speed
    bool press; // 1 if press
    bool moved; // 1 if |old_x - new_x| > 20 while pressed

private:
    bool is_moved (point p) const;
    direction_t get_direction (point p) const;
    void unpressed (view * vv, bool & need_redraw);
    void pressed (point p, view * vv, bool & need_redraw);
};

#endif

