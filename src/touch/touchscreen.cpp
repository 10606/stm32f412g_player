#include "touchscreen.h"

#include "moving.h"
#include "ts_touchscreen.h"
#include <stdlib.h>

uint32_t touch_tick_counter = 0;

//TODO calibrate
struct e
{
    static const int32_t left = 0;
    static const int32_t right = 240;
    static const int32_t top = 30;
    static const int32_t bottom = 190;

    static const constexpr point offset = {5, 5};
    static const constexpr point soft_offset = {2, 2};
    static const constexpr point direction_likelyhood = {4, 3}; // (x / y)

    static const  int32_t min_speed_cnt = 4;
    static const uint32_t max_speed_cnt = 20;
};

static inline int32_t nearest_to_zero (int32_t a, int32_t b)
{
    if (abs(a) < abs(b))
        return a;
    else
        return b;
}

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
    int32_t answer = (x - e::left) * 240 / (e::right - e::left);
    return clump(answer);
}

static inline int32_t normalize_y (int32_t y)
{
    int32_t answer = (y - e::top) * 240 / (e::bottom - e::top);
    return clump(answer);
}

static inline uint8_t is_delta_moved (point delta)
{
    return  (abs(delta.x) >= e::soft_offset.x) ||
            (abs(delta.y) >= e::soft_offset.y);
}

touch_state::touch_state () :
    press(0),
    moved(0)
{
    ts_ft6x06.init();
}

uint8_t touch_state::is_moved (point p) const
{
    return  (abs(old.x - p.x) >= e::offset.x) ||
            (abs(old.y - p.y) >= e::offset.y);
}

direction_t touch_state::get_direction (point p) const
{
    //if (!is_moved(ots, x, y))
    //    return direction_t::non;
    point diff = {p.x - old.x, p.y - old.y}; 
    if (abs(diff.x) * e::direction_likelyhood.x <= 
        abs(diff.y) * e::direction_likelyhood.y)
    {
        if (diff.y == 0)
            return direction_t::non;
        if (diff.y < 0)
            return direction_t::up;
        else
            return direction_t::down;
    }

    if (abs(diff.y) * e::direction_likelyhood.x <= 
        abs(diff.x) * e::direction_likelyhood.y)
    {
        if (diff.x == 0)
            return direction_t::non;
        if (diff.x < 0)
            return direction_t::left;
        else
            return direction_t::right;
    }

    return direction_t::non;
}


void touch_state::unpressed (view * vv, uint8_t * need_redraw)
{
    if (!press)
        return;

    int32_t multiplier = 16 / touch_tick_counter;
    dolg.x *= multiplier;
    dolg.y *= multiplier;
    if (abs(dolg.x) / e::offset.x <= e::min_speed_cnt)
        dolg.x = 0;
    if (abs(dolg.y) / e::offset.y <= e::min_speed_cnt)
        dolg.y = 0;
    bool flag = 1;
 
    direction_t direction = get_direction({old.x + dolg.x, old.y  + dolg.y});
    
    for (uint32_t cnt = 0; cnt < e::max_speed_cnt && flag; ++cnt)
    {
        if (!is_delta_moved(dolg))
            break;
        flag = 0;
        int32_t * dolg_v;
        switch (direction)
        {
        case direction_t::non:
            continue;
        case direction_t::up:
        case direction_t::down:
            dolg_v = &dolg.y;
            break;
        case direction_t::left:
        case direction_t::right:
            dolg_v = &dolg.x;
            break;
        }
        uint32_t offset = state.do_move(direction, abs(*dolg_v), 1, vv, need_redraw);
        *dolg_v -= nearest_to_zero(*dolg_v, offset);
        moved = 1;
        flag = 1;
    }

    if (!moved)
        state.touch_region(vv, need_redraw);

    press = 0;
    moved = 0;
}

void touch_state::pressed (point p, view * vv, uint8_t * need_redraw)
{
    if (press)
    {
        int32_t offset;
        direction_t direction = get_direction(p);
        
        if (is_moved(p))
            moved = 1;
        else
            direction = direction_t::non;

        switch (direction)
        {
        case direction_t::non:
            dolg.x = p.x - old.x;
            dolg.y = p.y - old.y;
            return;
        case direction_t::up:
        case direction_t::down:
            offset = state.do_move(direction, abs(p.y - old.y), 0, vv, need_redraw);
            old.x = p.x;
            old.y += nearest_to_zero(offset, p.y - old.y);
            dolg.x = 0;
            dolg.y = offset;
            break;
        case direction_t::left:
        case direction_t::right:
            offset = state.do_move(direction, abs(p.x - old.x), 0, vv, need_redraw);
            old.x += nearest_to_zero(offset, p.x - old.x);
            old.y = p.y;
            dolg.x = offset;
            dolg.y = 0;
            break;
        }
        moved = 1;
    }
    else
    {
        state.start = p;
        old.x = p.x;
        old.y = p.y;
        dolg.x = 0;
        dolg.y = 0;
        state.direction_mask = 0;
        press = 1;
        touch_tick_counter = 0;
    }
}

void touch_state::touch_check (view * vv, uint8_t * need_redraw)
{
    TS_StateTypeDef ts_state = {0};
    ts_ft6x06.ts_touch_detect(&ts_state);
    if (ts_state.touchDetected)
    {
        point p;
        p.x = normalize_x(ts_state.touchX[0]);
        p.y = normalize_y(ts_state.touchY[0]);
        pressed(p, vv, need_redraw);
    }
    else
    {
        unpressed(vv, need_redraw);
    }
}

