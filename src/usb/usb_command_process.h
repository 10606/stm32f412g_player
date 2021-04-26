#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

struct usb_process_t
{
    uint32_t usb_process (view * vv, bool & need_redraw);
    void receive_callback (volatile uint8_t * buf, uint32_t len);
    void clear ();

    uint8_t buffer[4];
    uint32_t start = 0;
    uint32_t end = 0;
};

extern usb_process_t usb_process_v;

#endif

