#include "player.h"

#include <stdint.h>
#include "audio.h"
#include "mp3.h"
#include "joystick.h"
#include "touchscreen.h"
#include "view.h"
#include "usb_command_process.h"
#include "main.h"

void main_player ()
{ 
    if (audio_start())
    {
        deinit_mad();
        return;
    }
  
    uint8_t need_redraw = 1;
    old_touch_state touch_state = {0};
    
    while (1)
    {
        uint32_t ret;
        if ((ret = audio_process(&need_redraw)))
        {
            if ((ret != audio_error_eof) &&
                (ret != audio_error_notready))
                ; //break;
        }
        check_buttons();
      
        if (need_redraw)
        {
            need_redraw = 0;
            display_view(&viewer, &need_redraw);
        }
        if ((ret = process_view(&viewer, &need_redraw)))
            ;//break;
        if (usb_process(&viewer, &need_redraw))
            ; //break;
        touch_check(&touch_state, &viewer, &need_redraw);
        
        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            deinit_mad();
            break;
        }
    }
}

