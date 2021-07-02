#include "moving.h"

#include "view.h"
#include "display_offsets.h"

struct offsets
{
    static const int32_t limit = display::offsets::line - 5;
    static const int32_t add = display::offsets::line - 5;
    static const int32_t limit_speed = 10;
    static const int32_t add_speed = 10;
};

uint32_t touch_processing::touch_region (view * vv)
{
    if (start.y < static_cast <int32_t> (display::offsets::list))
    {
        if (start.x < 120)
            return vv->play_pause();
        else
            return vv->to_end_and_pause();
    }
    else
        return vv->process_right();
}

uint32_t touch_processing::move_left_right
(
    int32_t & ans,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t direction // 0 left, 1 - right
)
{
    static uint32_t (view::* process_view_do[2]) () =
    {
        &view::process_center,
        &view::process_left
    };
    
    if ((direction_mask & (1 << direction)) || 
        (!speed && (offset < offsets::limit)) ||
        (speed && (offset < offsets::limit_speed)))
    {
        ans = 0;
        return 0;
    }
    direction_mask = 1 << direction;
    uint32_t ret = (vv->*process_view_do[direction])();
    ans = speed? offsets::add_speed : offsets::add;
    ans = direction? ans : -ans;
    return ret;
}

uint32_t touch_processing::move_left (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_left_right(ans, offset, speed, vv, 0);
}

uint32_t touch_processing::move_right (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_left_right(ans, offset, speed, vv, 1);
}

uint32_t touch_processing::move_up_down
(
    int32_t & ans,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t direction // 0 - down, 1 - up 
)
{
    if ((!speed && offset < offsets::limit) ||
        (speed && (offset < offsets::limit_speed)))
    {
        ans = 0;
        return 0;
    }
    direction_mask = 0;
    // reverse direction on pl_list and playlist
    uint8_t process_view_direction = (vv->state == state_t::song)? direction : 1 - direction;
    uint32_t ret = vv->process_next_prev(process_view_direction);
    ans = speed? offsets::add_speed : offsets::add;
    ans = direction? -ans : ans;
    return ret;
}

uint32_t touch_processing::move_up (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_up_down(ans, offset, speed, vv, 1);
}

uint32_t touch_processing::move_down (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_up_down(ans, offset, speed, vv, 0);
}

uint32_t touch_processing::do_move 
(
    int32_t & ans,
    direction_t direction,
    int32_t offset, 
    char speed, 
    view * vv
)
{
    uint32_t (touch_processing::* do_move_impl) 
    (
        int32_t & ans,
        int32_t offset, 
        char speed, 
        view * vv
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
        ans = 0;
        return 0;
    }
    return (this->*do_move_impl)(ans, offset, speed, vv);
}

