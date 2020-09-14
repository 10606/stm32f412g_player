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
#include "main.h"
#include "audio.h"
#include "play.h"
#include "view.h"
#include "FAT.h"

view viewer;

static void SystemClock_Config (void);
static void Error_Handler (void);
void audio_init ();
void AudioPlay_demo ();
void audio_destruct ();
DSTATUS SD_initialize (BYTE);

#include "stm32f4xx_hal_gpio.h"
#define JOY_LEFT_Pin        GPIO_PIN_15
#define JOY_LEFT_GPIO_Port  GPIOF
#define JOY_UP_Pin          GPIO_PIN_0
#define JOY_UP_GPIO_Port    GPIOG
#define JOY_DOWN_Pin        GPIO_PIN_1
#define JOY_DOWN_GPIO_Port  GPIOG

void init_base () //joystick, led, LCD
{
    HAL_Init();
    SystemClock_Config();
    
    //joystick init
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = JOY_UP_Pin|JOY_DOWN_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    }

    //LED3 init
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_GPIOE_CLK_ENABLE();
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    }
    
    BSP_LCD_Init();
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);
}

void init_fs (char (* path)[12], uint32_t len)
{
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

    while (BSP_SD_IsDetected() != SD_PRESENT)
    {
        BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"Please insert SD Card", CENTER_MODE);
    }

    if (!SD_initialize((BYTE)0))
    {
        Storage_Init();
        /* Open filesystem */
        global_info.sector_size = 512;
        if (init_fatfs())
        {
            BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not initialized...", 0);
            Error_Handler();
        }
    }
    else
    {
        Error_Handler();
    }
}

void init_audio (char (* path)[12], uint32_t len)
{
    audio_init();

    /* Get the .PLB file names on root directory */
    uint32_t ret = init_view(&viewer, path, len, &buffer_ctl);
    if (ret)
    {
        BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"No PLB files...", CENTER_MODE);
        Error_Handler();
    }
    
    /*
    memcpy(path[len - 1], pDirectoryFiles[index], 12);
    open(&fd_plv, path, len);
    init_playlist_view(&plv, &fd_plv);
  
    open(&fd_pl, path, len);
    init_playlist(&pl, &fd_pl);
    */
}

void init_timer ()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->SMCR &= ~TIM_SMCR_SMS;
    TIM2->PSC = 359;//719;
    TIM2->ARR = 24999;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;
}

void init (char (* path)[12], uint32_t len, uint32_t index)
{
    init_base();
    init_fs(path, len);
    init_audio(path, len);
    init_timer();
}

int main (void)
{
    char path[10][12] = {"MEDIA      "};
    init(path, 1, 0);

    uint32_t counter = 0;
    while (1)
    {
        counter = 0;
        /*
        while (counter < ubNumberOfFiles)
        {
            memcpy(path[1], pDirectoryFiles[counter], 12);
            open_playlist(&plv, path, 2);
        */
            AudioPlay_demo();
            counter++;
        //}
    }
    audio_destruct();
}

static void Error_Handler (void)
{
    BSP_LED_On(LED3);
    while(1)
    {}
}

static void SystemClock_Config (void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    HAL_StatusTypeDef ret = HAL_OK;

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
    if (ret != HAL_OK)
    {
        while (1) {} 
    }

    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
    if (ret != HAL_OK)
    {
        while (1) {}
    }
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
    static uint32_t debounce_time = 0;

    if (GPIO_Pin == BUTTON_WAKEUP)
    {
        /* Prevent debounce effect for user key */
        if ((HAL_GetTick() - debounce_time) > 50)
        {
            debounce_time = HAL_GetTick();
        }
    }
    else if (GPIO_Pin == SD_DETECT_PIN)
    {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    while (1)
    {}
}
#endif

