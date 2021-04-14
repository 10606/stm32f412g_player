#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

uint32_t usb_process (view * vv, uint8_t * need_redraw);
void receive_callback (volatile uint8_t * buf, uint32_t len);

#endif

