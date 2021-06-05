#include "joystick.h"

#include "stm32412g_discovery.h"
joystick_state_t joystick_state;

void joystick_state_t::check_buttons ()
{
    static JOYState_TypeDef joy_bsp_states[5] = 
    {
        JOY_UP,
        JOY_DOWN,
        JOY_LEFT,
        JOY_RIGHT,
        JOY_SEL
    };
    
    JOYState_TypeDef joy_state = BSP_JOY_GetState();
  
    for (uint32_t i = 0; i != 5; ++i)
    {
        if (joy_state == joy_bsp_states[i])
            pressed[i] = 1;
        else
            pressed[i] = 0;
    }
    
    visited = 1;
}

bool joystick_state_t::check_button_state (uint32_t joy_button)
{
    static uint8_t const cost[3] = {1, 8, 1}; // first second next
    bool ans = process[joy_button] >= cost[prev_processed[joy_button]];
    if (ans)
    {
        process[joy_button] -= cost[prev_processed[joy_button]];
        prev_processed[joy_button]++;
        if (prev_processed[joy_button] > 2)
            prev_processed[joy_button] = 2;
    }
    return ans;
}

uint32_t joystick_state_t::joystick_check (view & vv, bool & need_redraw)
{
    check_buttons();
    static uint32_t (view::* const process_view_do[joystick_states_cnt]) (bool &) = 
    {
        &view::process_up,
        &view::process_down,
        &view::process_left,
        &view::process_right,
        &view::process_center
    };
    static enum joystick_buttons buttons[joystick_states_cnt] = 
    {
        joy_button_up,
        joy_button_down,
        joy_button_left,
        joy_button_center,
        joy_button_right
    };
    for (uint32_t i = 0; i != joystick_states_cnt; ++i)
    {
        if (check_button_state(buttons[i]))
        {
            uint32_t ret = (vv.*process_view_do[i])(need_redraw);
            if (ret)
                return ret;
        }
    }
    return 0;
}

