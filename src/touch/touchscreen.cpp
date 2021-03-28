#include "touchscreen.h"

#include "util.h"
#include "moving.h"
#include "ts_touchscreen.h"
#include <stdlib.h>

touchscreen_ft6x06 ts_ft6x06;

uint32_t touch_tick_counter = 0;

//TODO calibrate
const int32_t e_left = 0;
const int32_t e_right = 240;
const int32_t e_top = 30;
const int32_t e_bottom = 190;

//TODO calibrate
const int32_t e_x_offset = 5;
const int32_t e_y_offset = 5;
const int32_t e_x_soft_offset = 2;
const int32_t e_y_soft_offset = 2;
const int32_t e_direction_likelyhood_v = 4;
const int32_t e_direction_likelyhood_d = 3;

const uint32_t e_speed_multiplier = 1600;
const int32_t e_min_speed_cnt = 4;
const uint32_t e_max_speed_cnt = 20;

static inline int32_t clump (int32_t a)
{
    if (a < 0)
        return 0;
    if (a > 240)
        return 240;
    return a;
}

static inline int32_t normalize_x (int32_t x)
{
    int32_t answer = (x - e_left) * 240 / (e_right - e_left);
    return clump(answer);
}

static inline int32_t normalize_y (int32_t y)
{
    int32_t answer = (y - e_top) * 240 / (e_bottom - e_top);
    return clump(answer);
}

static inline uint8_t is_delta_moved (int32_t delta_x, int32_t delta_y)
{
    return  (abs(delta_x) >= e_x_soft_offset) ||
            (abs(delta_y) >= e_y_soft_offset);
}

static inline uint8_t is_moved (old_touch_state const * ots, int32_t x, int32_t y)
{
    return  (abs(ots->old_x - x) >= e_x_offset) ||
            (abs(ots->old_y - y) >= e_y_offset);
}

static inline direction_t get_direction (old_touch_state const * ots, int32_t x, int32_t y)
{
    //if (!is_moved(ots, x, y))
    //    return NON_DIRECTION;
    int32_t diff_x = x - ots->old_x; 
    int32_t diff_y = y - ots->old_y; 
    if (abs(diff_x) * e_direction_likelyhood_v <= abs(diff_y) * e_direction_likelyhood_d)
    {
        if (diff_y == 0)
            return NON_DIRECTION;
        if (diff_y < 0)
            return UP_DIRECTION;
        else
            return DOWN_DIRECTION;
    }

    if (abs(diff_y) * e_direction_likelyhood_v <= abs(diff_x) * e_direction_likelyhood_d)
    {
        if (diff_x == 0)
            return NON_DIRECTION;
        if (diff_x < 0)
            return LEFT_DIRECTION;
        else
            return RIGHT_DIRECTION;
    }

    return NON_DIRECTION;
}


static void unpressed (old_touch_state * ots, view * vv, uint8_t * need_redraw)
{
    if (!ots->pressed)
        return;

    int32_t multiplier = 16 / touch_tick_counter;
    ots->dolg_x *= multiplier;
    ots->dolg_y *= multiplier;
    if (abs(ots->dolg_x) / e_x_offset <= e_min_speed_cnt)
        ots->dolg_x = 0;
    if (abs(ots->dolg_y) / e_y_offset <= e_min_speed_cnt)
        ots->dolg_y = 0;
    char flag = 1;
    uint32_t offset;
 
    direction_t direction = get_direction(ots, ots->old_x + ots->dolg_x, ots->old_y  + ots->dolg_y);
    
    for (uint32_t cnt = 0; cnt < e_max_speed_cnt && flag; ++cnt)
    {
        if (!is_delta_moved(ots->dolg_x, ots->dolg_y))
            break;
        flag = 0;
        switch (direction)
        {
        case UP_DIRECTION:
        case DOWN_DIRECTION:
            offset = do_move[direction](ots, abs(ots->dolg_y), 1, vv, need_redraw);
            ots->dolg_y -= nearest_to_zero(ots->dolg_y, offset);
            ots->moved = 1;
            flag = 1;
            break;
        case LEFT_DIRECTION:
        case RIGHT_DIRECTION:
            offset = do_move[direction](ots, abs(ots->dolg_x), 1, vv, need_redraw);
            ots->dolg_x -= nearest_to_zero(ots->dolg_x, offset);
            ots->moved = 1;
            flag = 1;
            break;
        case NON_DIRECTION:
            break;
        }
    }

    if (!ots->moved)
        touch_region(ots, vv, need_redraw);

    ots->pressed = 0;
    ots->moved = 0;
}

static void pressed (int32_t x1, int32_t y1, old_touch_state * ots, view * vv, uint8_t * need_redraw)
{
    if (ots->pressed)
    {
        int32_t offset;
        direction_t direction = get_direction(ots, x1, y1);
        
        if (is_moved(ots, x1, y1))
            ots->moved = 1;
        else
            direction = NON_DIRECTION;

        switch (direction)
        {
        case UP_DIRECTION:
        case DOWN_DIRECTION:
            offset = do_move[direction](ots, abs(y1 - ots->old_y), 0, vv, need_redraw);
            ots->old_x = x1;
            ots->old_y += nearest_to_zero(offset, y1 - ots->old_y);
            ots->dolg_x = 0;
            ots->dolg_y = offset;
            ots->moved = 1;
            break;
        case LEFT_DIRECTION:
        case RIGHT_DIRECTION:
            offset = do_move[direction](ots, abs(x1 - ots->old_x), 0, vv, need_redraw);
            ots->old_x += nearest_to_zero(offset, x1 - ots->old_x);
            ots->old_y = y1;
            ots->dolg_x = offset;
            ots->dolg_y = 0;
            ots->moved = 1;
            break;
        case NON_DIRECTION:
            ots->dolg_x = x1 - ots->old_x;
            ots->dolg_y = y1 - ots->old_y;
            break;
        }
    }
    else
    {
        ots->start_x = x1;
        ots->start_y = y1;
        ots->old_x = x1;
        ots->old_y = y1;
        ots->dolg_x = 0;
        ots->dolg_y = 0;
        ots->direction_mask = 0;
        ots->pressed = 1;
        touch_tick_counter = 0;
    }
}

void touch_check (old_touch_state * ots, view * vv, uint8_t * need_redraw)
{
    TS_StateTypeDef ts_state = {0};
    ts_ft6x06.ts_touch_detect(&ts_state);
    if (ts_state.touchDetected)
    {
        int32_t x1 = normalize_x(ts_state.touchX[0]);
        int32_t y1 = normalize_y(ts_state.touchY[0]);
        pressed(x1, y1, ots, vv, need_redraw);
    }
    else
    {
        unpressed(ots, vv, need_redraw);
    }
}

