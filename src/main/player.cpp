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
    if (audio_ctl.audio_init())
        return;
  
    uint8_t need_redraw = 1;
    touch_state touch;
    
    while (1)
    {
        uint32_t ret;
        if ((ret = audio_ctl.audio_process(&need_redraw)))
            ;
        if ((ret = joystick_state.joystick_check(viewer(), &need_redraw)))
            ;
        if ((ret = usb_process(&viewer(), &need_redraw)))
            ; 
        touch.touch_check(&viewer(), &need_redraw);
        
        if (need_redraw)
        {
            need_redraw = 0;
            viewer().display(&need_redraw);
        }

        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            audio_ctl.audio_destruct();
            break;
        }
    }
}

