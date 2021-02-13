#include "sd_card_operation.h"

#include "stm32412g_discovery_sd.h"
#include "stdint.h"

uint32_t start_partition_sector = 1;
uint32_t read_sector (uint32_t sector_number, void * buffer)
{
    if (BSP_SD_ReadBlocks((uint32_t*)buffer,
                        (uint32_t) (sector_number),
                        1, SD_TIMEOUT) == MSD_OK)
    {
        /* wait until the read operation is finished */
        uint32_t tried = 20;
        while ((BSP_SD_GetCardState() != MSD_OK) && tried)
        {
            --tried;
        }
        if (tried != 0)
            return 0;
    }
    return 1;
}

static inline uint32_t sd_status ()
{
    if (BSP_SD_GetCardState() == MSD_OK)
        return 0;
    else
        return sd_card_state_error;
}

uint32_t sd_card_init ()
{
    if (BSP_SD_Init() == MSD_OK)
        return sd_status();
    return sd_card_init_error;
}


