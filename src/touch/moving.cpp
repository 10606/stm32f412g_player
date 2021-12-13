#include "moving.h"

#include "view.h"
#include "display_offsets.h"
#include "direction_t.h"

struct offsets
{
    static const int32_t limit = display::offsets::line - 5;
    static const int32_t add = display::offsets::line - 5;
    static const int32_t limit_speed = 10;
    static const int32_t add_speed = 10;
};

ret_code touch_processing::touch_region (view * vv)
{
    if (start.y < static_cast <int32_t> (display::offsets::list))
    {
        if (start.x < 120)
            return vv->play_pause();
        else
            return vv->to_end_and_pause();
    }
    else
    {
        if (vv->state != state_t::playlist)
            return vv->process_right();
        
        if (start.x < 80)
            return vv->process_right();
        else if (start.x < 160)
            return vv->set_jmp_pos();
        else
            return vv->set_next_song();
    }
}

ret_code touch_processing::move_left_right
(
    int32_t & ans,
    int32_t offset, 
    char speed, 
    view * vv, 
    directions::lr::type direction
)
{
    static ret_code (view::* process_view_do[2]) () =
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
    ret_code ret = (vv->*process_view_do[direction])();
    ans = speed? offsets::add_speed : offsets::add;
    ans = direction == directions::lr::right? ans : -ans;
    return ret;
}

ret_code touch_processing::move_left (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_left_right(ans, offset, speed, vv, directions::lr::left);
}

ret_code touch_processing::move_right (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_left_right(ans, offset, speed, vv, directions::lr::right);
}

ret_code touch_processing::move_up_down
(
    int32_t & ans,
    int32_t offset, 
    char speed, 
    view * vv, 
    directions::du::type direction
)
{
    if ((!speed && offset < offsets::limit) ||
        (speed && (offset < offsets::limit_speed)))
    {
        ans = 0;
        return 0;
    }
    direction_mask = 0;
    
    // reverse (down -> prev, up -> next) direction on pl_list and playlist
    directions::du::type cmp = ((vv->state == state_t::song)? directions::du::down : directions::du::up);
    directions::np::type process_view_direction = (direction == cmp? directions::np::next : directions::np::prev);
    
    ret_code ret = vv->process_next_prev(process_view_direction);
    ans = speed? offsets::add_speed : offsets::add;
    ans = direction == directions::du::up? -ans : ans;
    return ret;
}

ret_code touch_processing::move_up (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_up_down(ans, offset, speed, vv, directions::du::up);
}

ret_code touch_processing::move_down (int32_t & ans, int32_t offset, char speed, view * vv)
{
    return move_up_down(ans, offset, speed, vv, directions::du::down);
}

ret_code touch_processing::do_move 
(
    int32_t & ans,
    direction_n::type direction,
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
    case direction_n::left:
        do_move_impl = &touch_processing::move_left;
        break;
    case direction_n::right:
        do_move_impl = &touch_processing::move_right;
        break;
    case direction_n::up:
        do_move_impl = &touch_processing::move_up;
        break;
    case direction_n::down:
        do_move_impl = &touch_processing::move_down;
        break;
    default:
        ans = 0;
        return 0;
    }
    return (this->*do_move_impl)(ans, offset, speed, vv);
}

