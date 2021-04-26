#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "view.h"

enum joystick_buttons
{
    joy_button_up = 0,
    joy_button_down = 1,
    joy_button_left = 2,
    joy_button_right = 3,
    joy_button_center = 4,
    joystick_states_cnt = 5
};

struct joystick_state_t
{
    uint32_t joystick_check (view & vv, bool & need_redraw);

    uint8_t pressed[joystick_states_cnt];
    uint8_t process[joystick_states_cnt];
    uint8_t prev_processed[joystick_states_cnt];
    
private:
    void check_buttons ();
    uint8_t check_button_state (uint32_t joy_button);
};
extern joystick_state_t joystick_state;


#endif

