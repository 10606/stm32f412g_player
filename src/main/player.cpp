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
  
    while (1)
    {
        uint32_t ret;
        if ((ret = audio_ctl.audio_process()))
            ;
        if ((ret = joystick_state.joystick_check(viewer())))
            ;
        if ((ret = usb_process_v.usb_process(&viewer())))
            ; 
        if ((ret = touch.touch_check(&viewer())))
            ;
        
        if (audio_ctl.need_redraw)
        {
            audio_ctl.need_redraw = 0;
            viewer().display();
        }

        if (BSP_SD_IsDetected() != SD_PRESENT)
        {   
            HAL_Delay(1);
            if (BSP_SD_IsDetected() == SD_PRESENT)
                continue;
            audio_ctl.audio_destruct();
            break;
        }
    }
}

