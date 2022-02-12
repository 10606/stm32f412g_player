#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "view.h"

struct joystick_state_t
{
    joystick_state_t ()
    {
        for (uint32_t i = 0; i != joystick_states_cnt; ++i)
        {
            pressed[i] = 0;
            button_state[i] = 0;
            time[i] = 0;
        }
    }

    enum joystick_buttons
    {
        joy_button_up = 0,
        joy_button_down = 1,
        joy_button_left = 2,
        joy_button_right = 3,
        joy_button_center = 4,
        joystick_states_cnt = 5
    };

    ret_code joystick_check (view & vv);
    void on_timer ();

    uint8_t pressed[joystick_states_cnt];
    uint8_t old_pressed[joystick_states_cnt];
    uint8_t button_state[joystick_states_cnt];
    volatile uint8_t time[joystick_states_cnt];
    
private:
    void check_buttons ();
    bool check_button_state (uint32_t joy_button);
    
    static const uint8_t cost_2 = 8;
};
extern joystick_state_t joystick_state;


#endif

