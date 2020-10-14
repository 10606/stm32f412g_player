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
#include "display/display.h"
#include "audio.h"
#include "play.h"
#include "view.h"
#include "FAT.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

view viewer;

void SystemClock_Config (void);
void Error_Handler (void);
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
        __HAL_RCC_GPIOG_CLK_ENABLE();
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
    //BSP_LCD_Clear(LCD_COLOR_WHITE);
    display_err();
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);
    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
}

uint32_t init_fs (char (* path)[12], uint32_t len)
{
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

    while (BSP_SD_IsDetected() != SD_PRESENT)
    {
        //BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"Please insert SD Card", CENTER_MODE);
        display_err();
    }

    while (SD_initialize((BYTE)0))
    {}
    if (!SD_initialize((BYTE)0))
    {
        if (Storage_Init())
            return 1;
        /* Open filesystem */
        global_info.sector_size = 512;
        if (init_fatfs())
        {
            BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not initialized...", 0);
            return 2;
        }
    }
    else
    {
        return 3;
    }
    return 0;
}

uint32_t init_audio (char (* path)[12], uint32_t len)
{
    audio_init();

    /* Get the .PLB file names on root directory */
    uint32_t ret = init_view(&viewer, path, len, &buffer_ctl);
    if (ret)
    {
        BSP_LCD_DisplayStringAt(0, 112, (uint8_t*)"No PLB files...", CENTER_MODE);
        audio_destruct();
        destroy_view(&viewer);
        return ret;
    }
    
    /*
    memcpy(path[len - 1], pDirectoryFiles[index], 12);
    open(&fd_plv, path, len);
    init_playlist_view(&plv, &fd_plv);
  
    open(&fd_pl, path, len);
    init_playlist(&pl, &fd_pl);
    */
    return 0;
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

void init_usb ()
{
    HAL_GPIO_WritePin(USB_OTGFS_PPWR_EN_GPIO_Port, USB_OTGFS_PPWR_EN_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin : USB_OTGFS_PPWR_EN_Pin */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = USB_OTGFS_PPWR_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USB_OTGFS_PPWR_EN_GPIO_Port, &GPIO_InitStruct);
    
    /*Configure GPIO pins : USB_OTGFS_OVRCR_Pin */
    GPIO_InitStruct.Pin = USB_OTGFS_OVRCR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
   
    MX_USB_DEVICE_Init();

    NVIC_SetPriority(OTG_FS_IRQn, 1);
}

uint32_t init (char (* path)[12], uint32_t len, uint32_t index)
{
    uint32_t ret;
    //init_base();
    ret = init_fs(path, len);
    if (ret)
        return ret;
    ret = init_audio(path, len);
    if (ret)
        return ret;
    init_timer();
    init_usb();
    return 0;
}

int main (void)
{
    init_base();
    uint32_t counter = 0;
    while (1)
    {
        char path[10][12] = {"MEDIA      "};
        if (init(path, 1, 0))
        {
            continue;
        }

        counter = 0;
        AudioPlay_demo();
        counter++;

        audio_destruct();
        destroy_view(&viewer);
        //BSP_LCD_Clear(LCD_COLOR_WHITE);
    }
}

void Error_Handler (void)
{
    BSP_LED_On(LED3);
    while(1)
    {}
}

/*
void SystemClock_Config (void)
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
*/

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 72;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 3;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = 
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection = 
        RCC_PERIPHCLK_I2S_APB1 | RCC_PERIPHCLK_SDIO | RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 50;
    PeriphClkInitStruct.PLLI2S.PLLI2SM = 4;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
    PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
    PeriphClkInitStruct.I2sApb1ClockSelection = RCC_I2SAPB1CLKSOURCE_PLLI2S;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
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

    if (GPIO_Pin == GPIO_PIN_9)
    {
        HAL_PCDEx_BCD_VBUSDetect(&hpcd_USB_OTG_FS);
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    while (1)
    {}
}
#endif

