#include "joystick.h"

joystick_state_t joystick_state;

void check_buttons ()
{
    static JOYState_TypeDef joy_bsp_states[5] = 
    {
        JOY_UP,
        JOY_DOWN,
        JOY_LEFT,
        JOY_RIGHT,
        JOY_SEL
    };
    
    JOYState_TypeDef JoyState = JOY_NONE;
    JoyState = BSP_JOY_GetState();
  
    for (uint32_t i = 0; i != 5; ++i)
    {
        if (JoyState == joy_bsp_states[i])
            joystick_state.pressed[i] = 1;
        else
            joystick_state.pressed[i] = 0;
    }
}

uint8_t check_button_state (uint32_t joy_button)
{
    static uint8_t const cost[3] = {1, 8, 1}; // first second next
    uint8_t ans = joystick_state.process[joy_button] >= cost[joystick_state.prev_processed[joy_button]];
    if (ans)
    {
        joystick_state.process[joy_button] -= cost[joystick_state.prev_processed[joy_button]];
        joystick_state.prev_processed[joy_button]++;
        if (joystick_state.prev_processed[joy_button] > 2)
            joystick_state.prev_processed[joy_button] = 2;
    }
    return ans;
}

