/**
  ******************************************************************************
  * @file    Display/LCD_PicturesFromSDCard/Src/fatfs_storage.c
  * @author  MCD Application Team
  * @brief   This file includes the Storage (FatFs) driver for the STM32412G-DISCOVERY
  *          application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright © 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include "main.h"
#include "FAT.h"
#include "fatfs_storage.h"
#include <stdint.h>

uint32_t Storage_Init(void)
{
    BSP_SD_Init();
    return 0;
}

uint32_t Storage_GetDirectoryPLBFiles (char (* dir_name)[12], size_t len_name, char (* Files)[MAX_PLB_FILE_NAME])
{
    file_descriptor file;
    char name[12];
    file_descriptor dir;
    uint32_t index = 0;
    uint32_t res;

    res = open(&dir, dir_name, len_name);
  
    if (res != 0)
    {
        BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"No directory Media...", 0);
        return 0;
    }
    for (;;)
    {
        res = read_dir(&dir, &file, name);
        if (res != 0 || name[0] == 0)
            break;
        if (name[0] == '.')
            continue;

        if (!file.is_dir)
        {
            if (index < MAX_PLB_FILES)
            {
                if ((name[8]  == 'P') && 
                    (name[9]  == 'L') && 
                    (name[10] == 'B'))
                {
                    memcpy(Files[index], name, sizeof(name));
                    index++;
                }
            }
        }
    }
    return index;
}

