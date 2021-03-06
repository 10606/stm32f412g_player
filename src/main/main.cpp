#include "usb_command_process.h"
#include "ts_touchscreen.h"
#include "FAT.h"
#include "sd_card_operation.h"
#include "display_error.h"
#include "display_picture.h"
#include "player.h"
#include "audio.h"
#include "view.h"
#include "init.h"
#include <type_traits>

FAT_info_t FAT_info(512, start_partition_sector, read_sector);

ret_code init_fs ()
{
    while (sd_card_init());

    // mount filesystem
    if (FAT_info.init())
    {
        display::error("err init fatfs");
        return 2;
    }
    return 0;
}

ret_code init_audio (filename_t * path, uint32_t len)
{
    // get the .PLB file names from path directory 
    ret_code ret = viewer.init(path, len);
    if (ret)
    {
        display::error("err init view");
        return ret;
    }
    return 0;
}

ret_code init (filename_t * path, uint32_t len)
{
    display::start_image();
    
    ret_code ret;
    ret = init_fs();
    if (ret)
        return ret;
    ret = init_audio(path, len);
    if (ret)
        return ret;
    usb_process_v.clear();
    return 0;
}

int main (void)
{
    init_base();
    while (1)
    {
        filename_t path[] = {"MEDIA      "};
        if (init(path, std::extent_v <decltype(path)>))
        {
            continue;
        }

        main_player();
        viewer.reset();
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed (uint8_t * file, uint32_t line)
{
    while (1)
    {}
}
#endif

