#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

struct usb_process_t
{
    usb_process_t ();
    uint32_t usb_process (view * vv);
    void receive_callback (uint8_t * buf, uint32_t len); 
    
    void clear ();

    volatile uint8_t buffer[4];
    volatile uint32_t start;
    volatile uint32_t end;
};

extern usb_process_t usb_process_v;

#endif

