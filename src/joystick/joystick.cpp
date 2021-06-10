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
  
    visited = 0;
    for (uint32_t i = 0; i != 5; ++i)
        pressed[i] = (HAL_GPIO_ReadPin(joy_pins[i].port, joy_pins[i].pin) == GPIO_PIN_SET);
    visited = 1;
}

bool joystick_state_t::check_button_state (uint32_t joy_button)
{
    static uint8_t const cost[] = {1, 8, 1}; // first second next
    static uint32_t const count = std::extent_v <decltype(cost)>; 
    
    bool ans = process[joy_button] >= cost[prev_processed[joy_button]];
    if (ans)
    {
        process[joy_button] -= cost[prev_processed[joy_button]];
        prev_processed[joy_button]++;
        if (prev_processed[joy_button] >= count)
            prev_processed[joy_button] = count - 1;
    }
    return ans;
}

uint32_t joystick_state_t::joystick_check (view & vv, bool & need_redraw)
{
    check_buttons();
    static constexpr uint32_t (view::* const process_view_do[joystick_states_cnt]) (bool &) = 
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
            uint32_t ret = (vv.*process_view_do[i])(need_redraw);
            if (ret)
                return ret;
        }
    }
    return 0;
}

void joystick_state_t::on_timer ()
{
    if (visited)
    {
        visited = 0;
        for (uint8_t i = 0; i != joystick_states_cnt; ++i)
        {
            if (pressed[i])
            {
                process[i]++;
                pressed[i] = 0;
            }
            else
            {
                prev_processed[i] = 0;
                process[i] = 0;
            }
        }
    }
}

