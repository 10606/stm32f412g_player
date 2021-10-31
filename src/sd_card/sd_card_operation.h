#ifndef SD_CARD_OPERATION_H
#define SD_CARD_OPERATION_H

#include "stm32412g_discovery_sd.h"
#include <stdint.h>
#include "util.h"

#if defined(SDMMC_DATATIMEOUT)
#define SD_TIMEOUT SDMMC_DATATIMEOUT
#elif defined(SD_DATATIMEOUT)
#define SD_TIMEOUT SD_DATATIMEOUT
#else
#define SD_TIMEOUT 1 * 1000
#endif

#define sd_card_state_error 1001
#define sd_card_init_error 1002

extern ret_code start_partition_sector;
extern ret_code read_sector (uint32_t sector_number, void * buffer);
extern ret_code sd_card_init ();

#endif

