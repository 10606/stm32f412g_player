/**
  ******************************************************************************
  * @file    Display/LCD_PicturesFromSDCard/Src/stm32f4xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "main.h"
#include "stm32f4xx_it.h"

/* uSD handler declared in "stm32412g_discovery_sd.c" file */
extern I2S_HandleTypeDef haudio_i2s;
extern SD_HandleTypeDef uSdHandle;

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

void NMI_Handler (void)
{
}

void HardFault_Handler (void)
{
    while (1)
    {}
}

void MemManage_Handler (void)
{
    while (1)
    {}
}

void BusFault_Handler (void)
{
    while (1)
    {}
}

void UsageFault_Handler (void)
{
    while (1)
    {}
}

void SVC_Handler (void)
{
}

void DebugMon_Handler (void)
{
}

void PendSV_Handler (void)
{
}

void SysTick_Handler (void)
{
    HAL_IncTick();
}

void AUDIO_OUT_I2Sx_DMAx_IRQHandler (void)
{
    HAL_DMA_IRQHandler(haudio_i2s.hdmatx);
}

void AUDIO_IN_I2Sx_DMAx_IRQHandler (void)
{
    HAL_DMA_IRQHandler(haudio_i2s.hdmarx);
}

void SDIO_IRQHandler (void)
{
    HAL_SD_IRQHandler(&uSdHandle);
}

void DMA2_Stream3_IRQHandler (void)
{
    HAL_DMA_IRQHandler(uSdHandle.hdmarx);
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s/startup_stm32f429x.s).    */
/******************************************************************************/

joystick_state_t joystick_state;

void TIM2_IRQHandler (void)
{
    for (uint8_t i = 0; i != joystick_states_cnt; ++i)
    {
        if (joystick_state.pressed[i])
        {
            joystick_state.process[i]++;
            joystick_state.pressed[i] = 0;
        }
    }
    
    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_RESET)
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
    TIM2->SR &= ~TIM_SR_UIF;
}

