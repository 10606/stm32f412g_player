#ifndef USB_COMMAND_PROCESS_H
#define USB_COMMAND_PROCESS_H

#include <stdint.h>
#include "view.h"

struct usb_process_t
{
    usb_process_t ();
    uint32_t usb_process (view * vv);
    void receive_callback (uint8_t * buf, uint32_t len); 
    // FIXME bug (not repeatable every time)
    // 
    // how see bug
    //   1 - do nothing on stm32 (just press reset)
    //   2 - connect player_stm32f412g (command_sender)
    // ! 3 - see empty screen !
    //   4 - press R
    //   5 - see normal screen
    //   
    // what happens
    //   1 - command_sender send 0x0e to multiplex_server 
    //   2 - stm32 send SOMETHING to multiplex_server
    //   3 - multiplex_server send 0x0e to stm32
    //   4 - multiplex_server send SOMETHING to command_sender 
    // ! 5 - stm32 NOT process (received?) data !
    
    void clear ();

    volatile uint8_t buffer[4];
    volatile uint32_t start;
    volatile uint32_t end;
};

extern usb_process_t usb_process_v;

#endif

