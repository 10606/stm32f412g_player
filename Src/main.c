/**
  ******************************************************************************
  * @file    Display/LCD_PicturesFromSDCard/Src/main.c
  * @author  MCD Application Team
  * @brief   This file provides main program functions
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
#include "play.h"
#include "FAT.h"

/** @addtogroup STM32F4xx_HAL_Applications
  * @{
  */

/** @addtogroup LCD_PicturesFromSDCard
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//FATFS SD_FatFs;  /* File system object for SD card logical drive */
char SD_Path[4]; /* SD card logical drive path */
char* pDirectoryFiles[MAX_BMP_FILES];
uint8_t  ubNumberOfFiles = 0;
uint32_t uwBmplen = 0;

/* Buffer defined in internal SRAM memory */
uint8_t uwInternelBuffer[LCD_SCREEN_WIDTH*LCD_SCREEN_HEIGHT*RGB565_BYTE_PER_PIXEL];

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
void audio_init ();
void AudioPlay_demo (playlist_view * plv, playlist * _pl);
void audio_destruct ();
DSTATUS SD_initialize (BYTE);

/* Private functions ---------------------------------------------------------*/

#include "stm32f4xx_hal_gpio.h"
#define JOY_LEFT_Pin GPIO_PIN_15
#define JOY_LEFT_GPIO_Port GPIOF
#define JOY_UP_Pin GPIO_PIN_0
#define JOY_UP_GPIO_Port GPIOG
#define JOY_DOWN_Pin GPIO_PIN_1
#define JOY_DOWN_GPIO_Port GPIOG

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    uint32_t counter = 0;
    char path[10][12] = {"MEDIA      "};
    //char * path[10] = {str[0]};
  
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
   */

    
    
   
  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = JOY_UP_Pin|JOY_DOWN_Pin;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  }

  {
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = GPIO_PIN_2;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  }
  HAL_Init();


  /* Configure the system clock to 100 MHz */
  SystemClock_Config();
  
  /* Configure LED3 */
  BSP_LED_Init(LED3);
  
  /*##-1- Configure LCD ######################################################*/
  
  /* LCD Initialization */
  BSP_LCD_Init();
  
  /* Clear the LCD */
  //BSP_LCD_Clear(LCD_COLOR_WHITE);
  
  /* Configure Key Button */
  BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);
  
  /* SD Initialization */
  BSP_SD_Init();
  
  /* Set the font Size */
  BSP_LCD_SetFont(&Font16);
  /* Set the Text Color */
  BSP_LCD_SetTextColor(LCD_COLOR_RED);
  /* Set the Back Color */
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

  while (BSP_SD_IsDetected() != SD_PRESENT)
  {
    BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"Please insert SD Card", CENTER_MODE);
  }

  /* Clear the LCD */
  //BSP_LCD_Clear(LCD_COLOR_WHITE);

  /*##-2- Link the SD Card disk I/O driver ###################################*/
  if (!SD_initialize((BYTE)0))
  {
    Storage_Init();
    /*##-3- Initialize the Directory Files pointers (heap) ###################*/
    for (counter = 0; counter < MAX_BMP_FILES; counter++)
    {
      pDirectoryFiles[counter] = malloc(MAX_BMP_FILE_NAME);
      if (pDirectoryFiles[counter] == NULL)
      {
        BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"Cannot allocate memory", CENTER_MODE);
        Error_Handler();
      }
    }

    /* Open filesystem */
    global_info.sector_size = 512;
    if (init_fatfs())
    {
        BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not initialized...", 0);
        return 0;
    }

    /* Get the BMP file names on root directory */
    ubNumberOfFiles = Storage_GetDirectoryPLBFiles(path, 1, pDirectoryFiles);

    if (ubNumberOfFiles == 0)
    {
        for (counter = 0; counter < MAX_BMP_FILES; counter++)
        {
            free(pDirectoryFiles[counter]);
        }

        BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"No PLB files...", CENTER_MODE);
        Error_Handler();
    }
  }
  else
  {
    /* FatFs Initialization Error */
    Error_Handler();
  }
  
  audio_init();

  memcpy(path[1], pDirectoryFiles[0], 12);
  file_descriptor fd_plv;
  playlist_view plv;
  open(&fd_plv, path, 2);
  init_playlist_view(&plv, &fd_plv);
  
  file_descriptor fd_pl;
  playlist pl;
  open(&fd_pl, path, 2);
  init_playlist(&pl, &fd_pl);

    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        TIM2->SMCR &= ~TIM_SMCR_SMS;
        TIM2->PSC = 359;//719;
        TIM2->ARR = 24999;
        TIM2->DIER |= TIM_DIER_UIE;
        NVIC_EnableIRQ(TIM2_IRQn);
        TIM2->CR1 |= TIM_CR1_CEN;
    }
    
  /* Main infinite loop */
  while (1)
  {
    counter = 0;
    while (counter < ubNumberOfFiles)
    {
      /* Format the string */
      //BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)pDirectoryFiles[counter], CENTER_MODE);
      //sprintf ((char*)str, "Media/%-30.30s", pDirectoryFiles[counter]);
      //sprintf ((char*)str, "Media/%-100.100s", pDirectoryFiles[counter]);
      memcpy(path[1], pDirectoryFiles[counter], 12);
      open_playlist(&plv, path, 2);

      if (Storage_CheckPLBFile(path, &uwBmplen) == 0)
      {
        /* Open a file and copy its content to an internal buffer */
        //audio_init();
        AudioPlay_demo(&plv, &pl);
        audio_destruct();

        //BSP_LCD_Clear(LCD_COLOR_BLACK);
        counter++;
      }
    }
  }
  audio_destruct();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while(1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 100000000
  *            HCLK(Hz)                       = 100000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 200
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);

  if(ret != HAL_OK)
  {
    while(1) { ; } 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t debounce_time = 0;

  if(GPIO_Pin == BUTTON_WAKEUP)
  {
    /* Prevent debounce effect for user key */
    if((HAL_GetTick() - debounce_time) > 50)
    {
      debounce_time = HAL_GetTick();
    }
  }
  else if (GPIO_Pin == SD_DETECT_PIN)
  {
    /* SDDetectIT global variable set to notice SD_exti_demo changing state on SD_DETECT_PIN */
    //SDDetectIT = 1;
  }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
