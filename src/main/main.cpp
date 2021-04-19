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
extern "C" 
{
void SystemClock_Config (void);
void Error_Handler (void);
}
#include "lcd_display.h"
#include "stm32f4xx_hal_gpio.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usb_command_process.h"
#include "ts_touchscreen.h"
#include "FAT.h"
#include "sd_card_operation.h"
#include "display.h"
#include "player.h"
#include "audio.h"
#include "view.h"

FAT_info_t FAT_info;


#define joy_center_pin           GPIO_PIN_0
#define joy_center_gpio_port     GPIOA
#define joy_left_pin             GPIO_PIN_15
#define joy_right_pin            GPIO_PIN_14
#define joy_left_right_gpio_port GPIOF
#define joy_up_pin               GPIO_PIN_0
#define joy_down_pin             GPIO_PIN_1
#define joy_up_down_gpio_port    GPIOG


void init_usb ();

void init_base () //joystick, led, LCD, USB
{
    HAL_Init();
    SystemClock_Config();
    
    //joystick init
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_init = {0};
        GPIO_init.Mode = GPIO_MODE_INPUT;
        GPIO_init.Pull = GPIO_PULLDOWN;
        GPIO_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

        GPIO_init.Pin = joy_up_pin | joy_down_pin;
        HAL_GPIO_Init(joy_up_down_gpio_port, &GPIO_init);

        GPIO_init.Pin = joy_left_pin | joy_right_pin;
        HAL_GPIO_Init(joy_left_right_gpio_port, &GPIO_init);

        GPIO_init.Pin = joy_center_pin;
        HAL_GPIO_Init(joy_center_gpio_port, &GPIO_init);
    }

    //LED3 init
    {
        GPIO_InitTypeDef GPIO_init = {0};
        __HAL_RCC_GPIOE_CLK_ENABLE();
        GPIO_init.Pin = GPIO_PIN_2;
        GPIO_init.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_init.Pull = GPIO_PULLDOWN;
        GPIO_init.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOE, &GPIO_init);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    }
    
    display_init();
    display::start_image();
    init_usb();
}

uint32_t init_fs (char (* path)[12], uint32_t len)
{
    while (BSP_SD_IsDetected() != SD_PRESENT)
        display::start_image();

    while (sd_card_init())
    {}
    // open filesystem
    if (init_fatfs(&FAT_info, 512, start_partition_sector, read_sector))
    {
        display::error("err init fatfs");
        return 2;
    }
    return 0;
}

uint32_t init_audio (char (* path)[12], uint32_t len)
{
    // get the .PLB file names from path directory 
    uint32_t ret = viewer.init(path, len, &audio_ctl);
    if (ret)
    {
        display::error("err init view");
        viewer.reset();
        return ret;
    }
    return 0;
}

void init_timer ()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->SMCR &= ~TIM_SMCR_SMS;
    TIM2->PSC = 179;//359;//719;
    TIM2->ARR = 24999;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;
}

void init_usb ()
{
    HAL_GPIO_WritePin(USB_OTGFS_PPWR_EN_GPIO_Port, USB_OTGFS_PPWR_EN_Pin, GPIO_PIN_SET);

    // configure GPIO pin : USB_OTGFS_PPWR_EN_Pin
    GPIO_InitTypeDef GPIO_init = {0};
    GPIO_init.Pin = USB_OTGFS_PPWR_EN_Pin;
    GPIO_init.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_init.Pull = GPIO_NOPULL;
    GPIO_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USB_OTGFS_PPWR_EN_GPIO_Port, &GPIO_init);
    
    // configure GPIO pins : USB_OTGFS_OVRCR_Pin
    GPIO_init.Pin = USB_OTGFS_OVRCR_Pin;
    GPIO_init.Mode = GPIO_MODE_INPUT;
    GPIO_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOG, &GPIO_init);
   
    MX_USB_DEVICE_Init();

    NVIC_SetPriority(OTG_FS_IRQn, 1);
}

uint32_t init (char (* path)[12], uint32_t len)
{
    uint32_t ret;
    //init_base();
    ret = init_fs(path, len);
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
    init_timer();
    while (1)
    {
        char path[10][12] = {"MEDIA      "};
        if (init(path, 1))
        {
            continue;
        }

        main_player();
        viewer.reset();
    }
}

void Error_Handler (void)
{
    BSP_LED_On(LED3);
    while (1)
    {}
}

void SystemClock_Config (void)
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
        Error_Handler();
    RCC_ClkInitStruct.ClockType = 
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        Error_Handler();
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
        Error_Handler();
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

#ifdef  USE_FULL_ASSERT
void assert_failed (uint8_t * file, uint32_t line)
{
    while (1)
    {}
}
#endif

