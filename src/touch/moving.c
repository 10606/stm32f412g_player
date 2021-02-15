#include "touchscreen.h"
#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"

const int32_t offset_limit = line_offset - 5;
const int32_t offset_add = line_offset - 5;
const int32_t offset_limit_speed = 10;
const int32_t offset_add_speed = 10;

void touch_region 
(
    old_touch_state * ots,
    view * vv, 
    uint8_t * need_redraw
)
{
    if (ots->start_y < list_offset)
        process_view_play_pause (vv, need_redraw);
    else
        process_view_right(vv, need_redraw);
}

int32_t move_left_right
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw,
    uint8_t direction // 0 left, 1 - right
)
{
    uint32_t (* process_view_do[2]) (view * vv, uint8_t * need_redraw) =
    {
        process_view_center,
        process_view_left
    };
    enum direction_t dir[2] = 
    {
        LEFT_DIRECTION,
        RIGHT_DIRECTION
    };
    
    if ((ots->direction_mask & (1 << dir[direction])) || 
        (!speed && (offset < offset_limit)) ||
        (speed && (offset < offset_limit_speed)))
        return 0;
    ots->direction_mask = 1 << dir[direction];
    process_view_do[direction](vv, need_redraw);
    int32_t ans;
    ans = speed? offset_add_speed : offset_add;
    return direction? ans : -ans;
}

int32_t move_left 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    return move_left_right(ots, offset, speed, vv, need_redraw, 0);
}

int32_t move_right 
(
    old_touch_state * ots,
    int32_t offset,
    char speed, 
    view * vv,
    uint8_t * need_redraw
)
{
    return move_left_right(ots, offset, speed, vv, need_redraw, 1);
}

int32_t move_up_down
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw,
    uint8_t direction // 0 - down, 1 - up 
)
{
    if ((!speed && offset < offset_limit) ||
        (speed && (offset < offset_limit_speed)))
        return 0;
    ots->direction_mask = 0;
    int32_t ans;
    // reverse direction on D_PL_LIST D_PLAYLIST
    uint8_t process_view_direction = (vv->state == D_SONG)? direction : 1 - direction;
    process_view_up_down(vv, need_redraw, process_view_direction);
    ans = speed? offset_add_speed : offset_add;
    return direction? -ans : ans;
}

int32_t move_up 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    return move_up_down(ots, offset, speed, vv, need_redraw, 1);
}

int32_t move_down 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    return move_up_down(ots, offset, speed, vv, need_redraw, 0);
}

int32_t (* const do_move[5]) 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
) = 
{
    move_left,  // NON_DIRECTION
    move_left,  // LEFT_DIRECTION
    move_right, // RIGHT_DIRECTION
    move_up,    // UP_DIRECTION
    move_down   // DOWN_DIRECTION 
};

