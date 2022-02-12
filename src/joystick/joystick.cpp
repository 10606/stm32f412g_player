#include "joystick.h"

#include "stm32412g_discovery.h"
#include "init.h"
#include <type_traits>

joystick_state_t joystick_state;

void joystick_state_t::check_buttons ()
{
    struct joy_pins_t
    {
        GPIO_TypeDef* port;
        uint32_t pin;
    };
    
    static joy_pins_t joy_pins[] =
    {
        {joy_up_down_gpio_port, joy_up_pin},
        {joy_up_down_gpio_port, joy_down_pin},
        {joy_left_right_gpio_port, joy_left_pin},
        {joy_left_right_gpio_port, joy_right_pin},
        {joy_center_gpio_port, joy_center_pin},
    };
  
    for (uint32_t i = 0; i != joystick_states_cnt; ++i)
    {
        old_pressed[i] = pressed[i];
        pressed[i] = (HAL_GPIO_ReadPin(joy_pins[i].port, joy_pins[i].pin) == GPIO_PIN_SET);
    }
}

bool joystick_state_t::check_button_state (uint32_t joy_button)
{
    static uint8_t const cost[] = {1, cost_2, 1}; // first second next
    static uint32_t const count = std::extent_v <decltype(cost)>; 
    
    bool ans = pressed[joy_button] && 
               (time[joy_button] >= cost[button_state[joy_button]]);
    if (ans)
    {
        time[joy_button] = 0;
        button_state[joy_button] = button_state[joy_button] + 1;
        if (button_state[joy_button] >= count)
            button_state[joy_button] = count - 1;
    }
    else if (!pressed[joy_button])
    {
        button_state[joy_button] = 0;
    }
    return ans;
}

ret_code joystick_state_t::joystick_check (view & vv)
{
    check_buttons();
    static constexpr uint32_t (view::* const process_view_do[joystick_states_cnt]) () = 
    {
        &view::process_up,
        &view::process_down,
        &view::process_left,
        &view::process_right,
        &view::process_center
    };
    static constexpr enum joystick_buttons buttons[joystick_states_cnt] = 
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
            ret_code ret = (vv.*process_view_do[i])();
            if (ret)
                return ret;
        }
    }
    return 0;
}

void joystick_state_t::on_timer ()
{
    for (uint32_t i = 0; i != joystick_states_cnt; ++i)
    {
        if (time[i] < cost_2)
            time[i] = time[i] + 1;
    }
}

