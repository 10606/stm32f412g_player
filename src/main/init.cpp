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

#include "init.h"

#include "stm32f4xx_hal_gpio.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "lcd_display.h"

uint32_t const joy_center_pin           = GPIO_PIN_0;
GPIO_TypeDef* const joy_center_gpio_port     = GPIOA;
uint32_t const joy_left_pin             = GPIO_PIN_15;
uint32_t const joy_right_pin            = GPIO_PIN_14;
GPIO_TypeDef* const joy_left_right_gpio_port = GPIOF;
uint32_t const joy_up_pin               = GPIO_PIN_0;
uint32_t const joy_down_pin             = GPIO_PIN_1;
GPIO_TypeDef* const joy_up_down_gpio_port    = GPIOG;

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
    RCC_OscInitStruct.PLL.PLLN = 72; // 48 Mhz;   = 96 for 96 Mhz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 3;  // 48 Mhz;   =  4 for 96 Mhz
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

void init_timer ()
{
    RCC->APB1ENR = RCC->APB1ENR | RCC_APB1ENR_TIM2EN;
    TIM2->SMCR = TIM2->SMCR & ~TIM_SMCR_SMS;
    TIM2->PSC = 179;//359;//719;
    TIM2->ARR = 24999;
    TIM2->DIER = TIM2->DIER | TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 = TIM2->CR1 | TIM_CR1_CEN;
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
    CDC_Init_FS();

    NVIC_SetPriority(OTG_FS_IRQn, 1);
}

void init_joystick ()
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

void init_LEDs ()
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_init = {0};
    GPIO_init.Pin = GPIO_PIN_2;
    GPIO_init.Pull = GPIO_PULLUP;
    GPIO_init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_init);

    //BSP_LED_On(LED3);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
}
    
void init_base ()
{
    HAL_Init();
    SystemClock_Config();
    init_joystick();
    init_LEDs();
    display_init();
    init_usb();
    init_timer();
}

void Error_Handler (void)
{
    while (1);
}

