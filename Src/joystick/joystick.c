#include "joystick.h"

joystick_state_t joystick_state;

void check_buttons ()
{
    JOYState_TypeDef JoyState = JOY_NONE;
    JoyState = BSP_JOY_GetState();
  
    if (JoyState == JOY_UP)
        joystick_state.pressed[joy_button_up] = 1;
    else
        joystick_state.pressed[joy_button_up] = 0;
      
    if (JoyState == JOY_DOWN)
        joystick_state.pressed[joy_button_down] = 1;
    else
        joystick_state.pressed[joy_button_down] = 0;

    if (JoyState == JOY_RIGHT)
        joystick_state.pressed[joy_button_right] = 1;
    else
        joystick_state.pressed[joy_button_right] = 0;

    if (JoyState == JOY_LEFT)
        joystick_state.pressed[joy_button_left] = 1;
    else
        joystick_state.pressed[joy_button_left] = 0;

    if (JoyState == JOY_SEL)
        joystick_state.pressed[joy_button_center] = 1;
    else
        joystick_state.pressed[joy_button_center] = 0;
}

