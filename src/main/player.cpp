#include "player.h"

#include "stm32412g_discovery_sd.h"
#include <stdint.h>
#include "audio.h"
#include "joystick.h"
#include "touchscreen.h"
#include "view.h"
#include "mp3.h"
#include "usb_command_process.h"

void main_player ()
{ 
    if (audio_start())
        return;
  
    uint8_t need_redraw = 1;
    old_touch_state touch_state = {0};
    
    while (1)
    {
        uint32_t ret;
        if ((ret = audio_process(&need_redraw)))
        {
        }
        check_buttons();
      
        if ((ret = process_view(&viewer, &need_redraw)))
            ;//break;
        if (usb_process(&viewer, &need_redraw))
            ; //break;
        touch_check(&touch_state, &viewer, &need_redraw);
        
        if (need_redraw)
        {
            need_redraw = 0;
            display_view(&viewer, &need_redraw);
        }

        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            deinit_mad();
            break;
        }
    }
}

