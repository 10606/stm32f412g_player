#include "touchscreen.h"
#include "stm32412g_discovery_audio.h"
#include "view.h"

const int32_t offset_limit = 15;
const int32_t offset_add = 15;
const int32_t offset_add_speed = 5;

void touch_region (int32_t x, int32_t y, view * vv, uint8_t * need_redraw)
{
    if (vv->buffer_ctl->pause_status == 1)
    { // Pause is enabled, call Resume 
        BSP_AUDIO_OUT_Resume();
        vv->buffer_ctl->pause_status = 0;
    } 
    else
    { // Pause the playback 
        BSP_AUDIO_OUT_Pause();
        vv->buffer_ctl->pause_status = 1;
    }
}

int32_t move_left 
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw)
{
    if (!speed && offset < offset_limit)
        return 0;
    process_view_right(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
}

int32_t move_right 
(
    int32_t x, 
    int32_t y, 
    int32_t offset,
    char speed, view * vv,
    uint8_t * need_redraw
)
{
    if (!speed && offset < offset_limit)
        return 0;
    process_view_left(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
}

int32_t move_up 
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    if (!speed && offset < offset_limit)
        return 0;
    process_view_down(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
}

int32_t move_down 
(
    int32_t x, 
    int32_t y, 
    int32_t offset, 
    char speed, 
    view * vv, 
    uint8_t * need_redraw
)
{
    if (!speed && offset < offset_limit)
        return 0;
    process_view_up(vv, need_redraw);
    if (speed)
        return offset_add_speed;
    else
        return offset_add;
}


