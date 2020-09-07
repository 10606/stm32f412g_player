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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FAT.h"
#include "fatfs_storage.h"

/** @addtogroup STM32412G-DISCOVERY_FAT_FS
  * @{
  */

/** @defgroup FATFS_STORAGE
  * @brief This file includes the Storage (FatFs) driver for the STM32412G-DISCOVERY
  *        application.
  * @{
  */

/** @defgroup FATFS_STORAGE_Private_Types
  * @{
  */
/**
  * @}
  */

/** @defgroup FATFS_STORAGE_Private_Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup FATFS_STORAGE_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup FATFS_STORAGE_Private_Variables
  * @{
  */
uint8_t sector[512];
/*
FATFS fs;
FILINFO fno;
DIR dir;
FIL F;
*/
const uint8_t SlidesCheck[2] =
  {
    0x42, 0x4D
  };
uint32_t BytesRead = 0;

/**
  * @}
  */


/** @defgroup FATFS_STORAGE_Private_FunctionPrototypes
  * @{
  */
/**
  * @}
  */

/** @defgroup FATFS_STORAGE_Private_Functions
  * @{
  */

/**
  * @brief  SDCARD Initialization for FatFs
  * @param  None
  * @retval err : Error status (0=> success, 1=> fail)
  */
uint32_t Storage_Init(void)
{
  BSP_SD_Init();
  return 0;
}

/**
  * @brief  Open a file and copy its content to a buffer
  * @param  DirName: the Directory name to open
  * @param  FileName: the file name to open
  * @param  BufferAddress: A pointer to a buffer to copy the file to
  * @param  FileLen: the File lenght
  * @retval err: Error status (0=> success, 1=> fail)
  */
uint32_t Storage_CheckPLBFile(char (* BmpName)[12], uint32_t *FileLen)
{
  return 0;
}

/**
  * @brief  List up to 25 file on the root directory with extension .BMP
  * @param  None
  * @retval The number of the found files
  */
uint32_t Storage_GetDirectoryPLBFiles (char (* dir_name)[12], size_t len_name, char* Files[])
{
    file_descriptor file;
    char name[12];
    file_descriptor dir;
    uint32_t counter = 0, index = 0;
    uint32_t res;

    /* Open directory */
    res = open(&dir, dir_name, len_name);
  
    if (res == 0)
    {
        for (;;)
        {
            res = read_dir(&dir, &file, name);
            if (res != 0 || name[0] == 0)
                break;
            if (name[0] == '.')
                continue;

            //BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)name, 0);

            if (!file.is_dir)
            {
                counter = 7;

                if (index < MAX_BMP_FILES)
                {
                    if (((name[counter + 1] == 'P') && 
                         (name[counter + 2] == 'L') && 
                         (name[counter + 3] == 'B')) ||
                        ((name[counter + 1] == 'p') && 
                         (name[counter + 2] == 'l') && 
                         (name[counter + 3] == 'b')))
                    {
                        //BSP_LCD_DisplayStringAt(0, 182, (uint8_t*)"find raw file", 0);
                        if(sizeof(name) <= (MAX_BMP_FILE_NAME + 2))
                        {
                            //BSP_LCD_DisplayStringAt(0, 192, (uint8_t*)"find raw file OK", 0);
                            sprintf (Files[index], "%s", name);
                            index++;
                        }
                    }
                }
                counter = 0;
            }
        }
    }
    else
    {
        BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"No directory Media...", 0);
    }
    return index;
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length.
  * @retval  0: pBuffer1 identical to pBuffer2
  *          1: pBuffer1 differs from pBuffer2
  */
uint8_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return 1;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
