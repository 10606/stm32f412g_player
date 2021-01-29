#include "touchscreen.h"

#include "util.h"
#include "moving.h"
#include "stm32412g_discovery_ts.h"
#include <stdlib.h>

//TODO calibrate
const int32_t e_left = 0;
const int32_t e_right = 240;
const int32_t e_top = 30;
const int32_t e_bottom = 190;

//TODO calibrate
const int32_t e_x_offset = 10;
const int32_t e_y_offset = 10;
const int32_t e_x_soft_offset = 2;
const int32_t e_y_soft_offset = 2;
const int32_t e_direction_likelyhood = 2;

const uint32_t e_speed_multiplier = 1600;
const int32_t e_min_speed_cnt = 4;
const uint32_t e_max_speed_cnt = 20;

int32_t clump (int32_t a)
{
    if (a < 0)
        return 0;
    if (a > 240)
        return 240;
    return a;
}

int32_t normalize_x (int32_t x)
{
    int32_t answer = (x - e_left) * 240 / (e_right - e_left);
    return clump(answer);
}

int32_t normalize_y (int32_t y)
{
    int32_t answer = (y - e_top) * 240 / (e_bottom - e_top);
    return clump(answer);
}

uint8_t is_delta_moved (int32_t delta_x, int32_t delta_y)
{
    return  (abs(delta_x) >= e_x_soft_offset) ||
            (abs(delta_y) >= e_y_soft_offset);
}

uint8_t is_moved (old_touch_state const * ots, int32_t x, int32_t y)
{
    return  (abs(ots->old_x - x) >= e_x_offset) ||
            (abs(ots->old_y - y) >= e_y_offset);
}

direction_t get_direction (old_touch_state const * ots, int32_t x, int32_t y)
{
    //if (!is_moved(ots, x, y))
    //    return NON_DIRECTION;
    int32_t diff_x = x - ots->old_x; 
    int32_t diff_y = y - ots->old_y; 
    if (abs(diff_x) * e_direction_likelyhood <= abs(diff_y))
    {
        if (diff_y < 0)
            return UP_DIRECTION;
        else
            return DOWN_DIRECTION;
    }

    if (abs(diff_y) * e_direction_likelyhood <= abs(diff_x))
    {
        if (diff_x < 0)
            return LEFT_DIRECTION;
        else
            return RIGHT_DIRECTION;
    }

    return NON_DIRECTION;
}


void unpressed (old_touch_state * ots, view * vv, uint8_t * need_redraw)
{
    if (!ots->pressed)
        return;

    uint32_t delta_time = HAL_GetTick() - ots->time;
    int32_t multiplier = e_speed_multiplier / delta_time;
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
        case DOWN_DIRECTION:
            offset = move_down(ots->start_x, ots->start_y, ots->dolg_y, 1, &ots->direction_mask, vv, need_redraw);
            ots->dolg_y = max_i(ots->dolg_y - offset, 0);
            ots->moved = 1;
            flag = 1;
            break;
        case UP_DIRECTION:
            offset = move_up(ots->start_x, ots->start_y, -ots->dolg_y, 1, &ots->direction_mask, vv, need_redraw);
            ots->dolg_y = min_i(ots->dolg_y + offset, 0);
            ots->moved = 1;
            flag = 1;
            break;
        case RIGHT_DIRECTION:
            offset = move_right(ots->start_x, ots->start_y, ots->dolg_x, 1, &ots->direction_mask, vv, need_redraw);
            ots->dolg_x = max_i(ots->dolg_x - offset, 0);
            ots->moved = 1;
            flag = 1;
            break;
        case LEFT_DIRECTION:
            offset = move_left(ots->start_x, ots->start_y, -ots->dolg_x, 1, &ots->direction_mask, vv, need_redraw);
            ots->dolg_x = min_i(ots->dolg_x + offset, 0);;
            ots->moved = 1;
            flag = 1;
            break;
        case NON_DIRECTION:
            break;
        }
    }

    if (ots->pressed && !ots->moved)
    {
        touch_region(ots->start_x, ots->start_y, &ots->direction_mask, vv, need_redraw);
        //  pause/resume
    }

    ots->pressed = 0;
    ots->moved = 0;
}


void touch_check (old_touch_state * ots, view * vv, uint8_t * need_redraw)
{
    TS_StateTypeDef  TS_State = {0};
    [[maybe_unused]] uint32_t ts_status = BSP_TS_GetState(&TS_State);
    if (TS_State.touchDetected)
    {
    
        int32_t x1 = normalize_x(TS_State.touchX[0]);
        int32_t y1 = normalize_y(TS_State.touchY[0]);
        
        if (ots->pressed)
        {
            int32_t offset;
            direction_t direction = get_direction(ots, x1, y1);
            
            if (is_moved(ots, x1, y1))
                ots->moved = 1;
            else
            {
                ots->dolg_x = x1 - ots->old_x;
                ots->dolg_y = y1 - ots->old_y;
                return;
            }
            
            switch (direction)
            {
            case UP_DIRECTION:
                offset = move_up(ots->start_x, ots->start_y, ots->old_y - y1, 0, &ots->direction_mask, vv, need_redraw);
                ots->old_x = x1;
                ots->old_y -= min(offset, ots->old_y - y1);
                ots->dolg_x = 0;
                ots->dolg_y = y1 - ots->old_y;
                break;
            case DOWN_DIRECTION:
                offset = move_down(ots->start_x, ots->start_y, y1 - ots->old_y, 0, &ots->direction_mask, vv, need_redraw);
                ots->old_x = x1;
                ots->old_y += min(offset, y1 - ots->old_y);
                ots->dolg_x = 0;
                ots->dolg_y = y1 - ots->old_y;
                break;
            case LEFT_DIRECTION:
                offset = move_left(ots->start_x, ots->start_y, ots->old_x - x1, 0, &ots->direction_mask, vv, need_redraw);
                ots->old_x -= min(offset, ots->old_x - x1);
                ots->old_y = y1;
                ots->dolg_x = x1 - ots->old_x;
                ots->dolg_y = 0;
                break;
            case RIGHT_DIRECTION:
                offset = move_right(ots->start_x, ots->start_y, x1 - ots->old_x, 0, &ots->direction_mask, vv, need_redraw);
                ots->old_x += min(offset, x1 - ots->old_x);
                ots->old_y = y1;
                ots->dolg_x = x1 - ots->old_x;
                ots->dolg_y = 0;
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
            ots->time = HAL_GetTick();
        }
    }
    else
    {
        unpressed(ots, vv, need_redraw);
    }
}

