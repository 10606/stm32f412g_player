#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

struct usb_process_t
{
    usb_process_t ();
    ret_code usb_process (view * vv);
    void receive_callback (volatile uint8_t * buf, uint32_t len); 
    
    void clear ();

private:
    uint32_t calc_need_rd (uint8_t first_byte);
    
    volatile uint8_t buffer[20 + sizeof(find_pattern)];
    volatile uint32_t start;
    volatile uint32_t end;
    volatile uint32_t end_buf;
    volatile uint32_t need_skip;
    volatile uint32_t need_rd;
    volatile bool has_interrupted;
    
    static const uint8_t cmd_find = 0x11;
};

extern usb_process_t usb_process_v;

#endif

