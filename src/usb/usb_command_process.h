#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"
#include "usb_commands.h"

struct usb_process_t
{
    usb_process_t ();
    ret_code usb_process (view * vv);
    void receive_callback (volatile uint8_t * buf, uint32_t len); 
    
    void clear ();

private:
    
    volatile uint8_t buffer[20 + std::max(sizeof(find_pattern), sizeof(position_t))];
    volatile uint32_t start;
    volatile uint32_t end;
    volatile uint32_t end_buf;
    volatile uint32_t need_skip;
    volatile uint32_t need_rd;
    volatile bool has_interrupted;
};

extern usb_process_t usb_process_v;

#endif

