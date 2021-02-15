#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "stm32412g_discovery.h"

enum joystick_buttons
{
    joy_button_up = 0,
    joy_button_down = 1,
    joy_button_left = 2,
    joy_button_right = 3,
    joy_button_center = 4,
    joystick_states_cnt = 5
}; // WARNING on change: process_view (view * vv, uint8_t * need_redraw)

typedef struct joystick_state_t
{
    uint8_t pressed[joystick_states_cnt];
    uint8_t process[joystick_states_cnt];
    uint8_t prev_processed[joystick_states_cnt];
} joystick_state_t;
extern joystick_state_t joystick_state;

void check_buttons ();


#endif

