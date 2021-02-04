#include "touchscreen.h"
#include "stm32412g_discovery_audio.h"
#include "view.h"
#include "display.h"

const int32_t offset_limit = line_offset - 5;
const int32_t offset_add = line_offset;
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

int32_t move_left 
(
    old_touch_state * ots,
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    if ((ots->direction_mask & (1 << LEFT_DIRECTION)) || 
        (!speed && (offset < offset_limit)) ||
        (speed && (offset < offset_add_speed)))
        return 0;
    ots->direction_mask = 1 << LEFT_DIRECTION;
    process_view_center(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
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
    if ((ots->direction_mask & (1 << RIGHT_DIRECTION)) || 
        (!speed && offset < offset_limit) ||
        (speed && (offset < offset_add_speed)))
        return 0;
    ots->direction_mask = 1 << RIGHT_DIRECTION;
    process_view_left(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
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
    if ((!speed && offset < offset_limit) ||
        (speed && (offset < offset_add_speed)))
        return 0;
    ots->direction_mask = 0;
    if (speed)
    {
        process_view_down(vv, need_redraw);
        return offset_add_speed;
    }
    else
    {
        if (offset >= 2 * offset_add)
        {
            uint32_t cnt = offset / offset_add;
            uint32_t new_pos = vv->plv.pos_selected + cnt;
            seek_playlist_view (&vv->plv, new_pos);
            *need_redraw = 1;
            return offset_add * cnt;
        }
        else
        {
            process_view_down(vv, need_redraw);
            return offset_add;
        }
    }
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
    if ((!speed && offset < offset_limit) ||
        (speed && (offset < offset_add_speed)))
        return 0;
    ots->direction_mask = 0;
    if (speed)
    {
        process_view_up(vv, need_redraw);
        return offset_add_speed;
    }
    else
    {
        if (offset >= 2 * offset_add)
        {
            uint32_t cnt = offset / offset_add;
            uint32_t new_pos = vv->plv.pos_selected + vv->plv.lpl.header.cnt_songs - cnt;
            seek_playlist_view (&vv->plv, new_pos);
            *need_redraw = 1;
            return offset_add * cnt;
        }
        else
        {
            process_view_up(vv, need_redraw);
            return offset_add;
        }
    }
}


