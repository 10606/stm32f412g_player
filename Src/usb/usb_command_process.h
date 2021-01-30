#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"
#include "usb_commands.h"

uint32_t usb_process (view * vv, uint8_t * need_redraw);

#endif

