#include "moving.h"

#include "view.h"
#include "display.h"

struct offsets
{
    static const int32_t limit = display::offsets::line - 5;
    static const int32_t add = display::offsets::line - 5;
    static const int32_t limit_speed = 10;
    static const int32_t add_speed = 10;
};

void touch_processing::touch_region 
(
    view * vv, 
    uint8_t * need_redraw
)
{
    if (start.y < static_cast <int32_t> (display::offsets::list))
        vv->play_pause(need_redraw);
    else
        vv->process_right(need_redraw);
}

int32_t touch_processing::move_left_right
(
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw,
    uint8_t direction // 0 left, 1 - right
)
{
    static uint32_t (view::* process_view_do[2]) (uint8_t * need_redraw) =
    {
        &view::process_center,
        &view::process_left
    };
    static uint8_t dir[2] = {0, 1};
    
    if ((direction_mask & (1 << dir[direction])) || 
        (!speed && (offset < offsets::limit)) ||
        (speed && (offset < offsets::limit_speed)))
        return 0;
    direction_mask = 1 << dir[direction];
    (vv->*process_view_do[direction])(need_redraw);
    int32_t ans;
    ans = speed? offsets::add_speed : offsets::add;
    return direction? ans : -ans;
}

int32_t touch_processing::move_left (int32_t offset, char speed, view * vv, uint8_t * need_redraw)
{
    return move_left_right(offset, speed, vv, need_redraw, 0);
}

int32_t touch_processing::move_right (int32_t offset, char speed, view * vv, uint8_t * need_redraw)
{
    return move_left_right(offset, speed, vv, need_redraw, 1);
}

int32_t touch_processing::move_up_down
(
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw,
    uint8_t direction // 0 - down, 1 - up 
)
{
    if ((!speed && offset < offsets::limit) ||
        (speed && (offset < offsets::limit_speed)))
        return 0;
    direction_mask = 0;
    int32_t ans;
    // reverse direction on pl_list and playlist
    uint8_t process_view_direction = (vv->state == state_t::song)? direction : 1 - direction;
    vv->process_up_down(need_redraw, process_view_direction);
    ans = speed? offsets::add_speed : offsets::add;
    return direction? -ans : ans;
}

int32_t touch_processing::move_up (int32_t offset, char speed, view * vv, uint8_t * need_redraw)
{
    return move_up_down(offset, speed, vv, need_redraw, 1);
}

int32_t touch_processing::move_down (int32_t offset, char speed, view * vv, uint8_t * need_redraw)
{
    return move_up_down(offset, speed, vv, need_redraw, 0);
}

int32_t touch_processing::do_move 
(
    direction_t direction,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    int32_t (touch_processing::* do_move_impl) 
    (
        int32_t offset, 
        char speed, 
        view * vv, 
        uint8_t * need_redraw
    );

    switch (direction)
    {
    case direction_t::left:
        do_move_impl = &touch_processing::move_left;
        break;
    case direction_t::right:
        do_move_impl = &touch_processing::move_right;
        break;
    case direction_t::up:
        do_move_impl = &touch_processing::move_up;
        break;
    case direction_t::down:
        do_move_impl = &touch_processing::move_down;
        break;
    default:
        return 0;
    }
    return (this->*do_move_impl)(offset, speed, vv, need_redraw);
}

