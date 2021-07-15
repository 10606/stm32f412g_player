#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

struct usb_process_t
{
    usb_process_t ();
    uint32_t usb_process (view * vv);
    void receive_callback (volatile uint8_t * buf, uint32_t len); 
    
    void clear ();

private:
    uint32_t calc_need_rd (uint8_t first_byte);
    
    volatile uint8_t buffer[20];
    volatile uint32_t start;
    volatile uint32_t end;
    volatile uint32_t end_buf;
    volatile uint32_t need_skip;
    volatile uint32_t need_rd;
};

extern usb_process_t usb_process_v;

#endif

