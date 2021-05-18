#ifndef INIT_H
#define INIT_H

extern "C" void SystemClock_Config (void);
extern "C" void Error_Handler (void);

void init_usb ();
void init_joystick ();
void init_LEDs ();
void init_timer ();

void init_base (); //joystick, led, LCD, USB, timer

#endif

