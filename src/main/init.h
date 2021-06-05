#ifndef INIT_H
#define INIT_H

#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>

extern uint32_t const joy_center_pin;
extern GPIO_TypeDef* const joy_center_gpio_port;
extern uint32_t const joy_left_pin;
extern uint32_t const joy_right_pin;
extern GPIO_TypeDef* const joy_left_right_gpio_port;
extern uint32_t const joy_up_pin;
extern uint32_t const joy_down_pin;
extern GPIO_TypeDef* const joy_up_down_gpio_port;

extern "C" void SystemClock_Config (void);
extern "C" void Error_Handler (void);

void init_usb ();
void init_joystick ();
void init_LEDs ();
void init_timer ();

void init_base (); //joystick, led, LCD, USB, timer

#endif

