
/**
  ******************************************************************************
  * @file    BSP/Src/audio.c 
  * @author  MCD Application Team
  * @brief   This example code shows how to use the audio feature in the 
  *          stm32412g_discovery driver
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include "display_playlist.h"
#include <stdio.h>
#include "stm32f4xx_it.h"


/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */ 



/* Private define ------------------------------------------------------------*/
#define str_size 2000


/*Since SysTick is set to 1ms (unless to set it quicker) */ 
/* to run up to 48khz, a buffer around 1000 (or more) is requested*/
/* to run up to 96khz, a buffer around 2000 (or more) is requested*/
//#define AUDIO_BUFFER_SIZE       8192  * 2
#define AUDIO_BUFFER_SIZE       12288 

#define AUDIO_DEFAULT_VOLUME    70

//TODO 2 buffers: for read from SD-CARD 
//  and for write
//  
//  write part and then read if needed;
//  
//  FIXME artefacts half write -> read
//  buffer [new part][old part]

/* Audio file size and start address are defined here since the audio file is
   stored in Flash memory as a constant table of 16-bit data */
#define AUDIO_FILE_SIZE               547732
#define AUDIO_START_OFFSET_ADDRESS    0            /* Offset relative to audio file header size */
#define AUDIO_FILE_ADDRESS            FLASH_DATA_ADDRESS   /* Audio file address */

#define HEADBAND_HEIGHT         72

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_INIT,    
  AUDIO_STATE_PLAYING,  
}AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
  BUFFER_OFFSET_NONE = 0,  
  BUFFER_OFFSET_HALF,  
  BUFFER_OFFSET_FULL,     
}BUFFER_StateTypeDef;

typedef struct {
  uint8_t buff[AUDIO_BUFFER_SIZE];
  uint32_t fptr;  
  BUFFER_StateTypeDef state;
}AUDIO_BufferTypeDef;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_BufferTypeDef  buffer_ctl;
static AUDIO_PLAYBACK_StateTypeDef  audio_state;
file_descriptor file;
//char (* file_name)[12];
playlist * pl;
playlist_view * plv;
//size_t file_name_len;
//static uint32_t  AudioStartAddress;
static uint32_t  AudioFileSize;
__IO uint32_t uwVolume = 20;
__IO uint32_t uwPauseEnabledStatus = 0;

static uint32_t AudioFreq[8] = {8000 ,11025, 16000, 22050, 32000, 44100, 48000, 96000};
static uint32_t *AudioFreq_ptr;
static JOYState_TypeDef JoyState = JOY_NONE;

/* Private function prototypes -----------------------------------------------*/
static void Audio_SetHint(void);
//static uint32_t GetData(void *pdata, uint32_t offset, uint8_t *pbuf, uint32_t NbrOfData);
static uint32_t GetData (file_descriptor * file, uint8_t *pbuf, uint32_t NbrOfData);
static uint32_t get_all_data (file_descriptor * _file, uint8_t *pbuf, uint32_t NbrOfData);
AUDIO_ErrorTypeDef AUDIO_Start();
uint8_t AUDIO_Process(void);
AUDIO_ErrorTypeDef AUDIO_Stop(void);

/* Private functions ---------------------------------------------------------*/


void audio_init ()
{
  AudioFreq_ptr = AudioFreq + 5; /*AF_44K*/
  uint8_t status = 0;
  uwPauseEnabledStatus = 0; /* 0 when audio is running, 1 when Pause is on */
  uwVolume = 80;
  
  Audio_SetHint();
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 160, (uint8_t *)"0123456789abcdefhijklmnoprsqtyuvxyz", LEFT_MODE);
  
  status = BSP_JOY_Init(JOY_MODE_GPIO);
  
  if (status != HAL_OK)
  {    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 100, (uint8_t *)"ERROR", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"Joystick init error", CENTER_MODE);
  }
  
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"   INIT CODEC    ", CENTER_MODE);
  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, uwVolume, *AudioFreq_ptr) == 0)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 100, (uint8_t *)"  AUDIO CODEC   OK  ", CENTER_MODE);
  }
  else
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 100, (uint8_t *)"  AUDIO CODEC  FAIL ", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)" Try to reset board ", CENTER_MODE);
  }
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"   INIT CODEC FINISH ", CENTER_MODE);
  
}

void audio_destruct ()
{
    BSP_AUDIO_OUT_DeInit();
}


/**
  * @brief  Audio Play demo
  * @retval None
  */
//void AudioPlay_demo (char (* _file_name)[12], size_t len)
void AudioPlay_demo (playlist_view * _plv, playlist * _pl)
{ 
  //file_name = _file_name;
  plv = _plv;
  pl = _pl;
  //file_name_len = len;
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 150, (uint8_t *)pl->song.song_name, LEFT_MODE);
  AudioFreq_ptr = AudioFreq + 5; /*AF_48K*/
  //uint8_t FreqStr[25] = {0};

  /* 
  Start playing the file from a circular buffer, once the DMA is enabled, it is 
  always in running state. Application has to fill the buffer with the audio data 
  using Transfer complete and/or half transfer complete interrupts callbacks 
  (DISCOVERY_AUDIO_TransferComplete_CallBack() or DISCOVERY_AUDIO_HalfTransfer_CallBack()...
  */
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       AUDIO START    ", CENTER_MODE);
  BSP_LCD_SetTextColor(LCD_COLOR_RED);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
  if (AUDIO_Start() == AUDIO_ERROR_IO)
  {
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 125, (uint8_t *)"       ERROR READ    ", CENTER_MODE);
  }
  
  /* Display the state on the screen */
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       PLAYING...     ", CENTER_MODE);
  
  //sprintf((char*)FreqStr,"       VOL:    %lu     ",uwVolume);
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 55, (uint8_t *)FreqStr, CENTER_MODE);
  
  //sprintf((char*)FreqStr,"      FREQ: %lu     ",*AudioFreq_ptr);
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 40, (uint8_t *)FreqStr, CENTER_MODE);
  
  char need_redraw = 0;
  display_playlist(plv, pl);
  /* Infinite loop */
  while(1)
  {
    /* IMPORTANT: AUDIO_Process() should be called within a periodic process */    
    AUDIO_Process();

    //HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    /* Get the Joystick State */
    JoyState = BSP_JOY_GetState();
    
    switch (JoyState)
    {
    case JOY_UP:
        joystick_state.pressed[joy_button_up] = 1;
        break;
        
    case JOY_DOWN:
        joystick_state.pressed[joy_button_down] = 1;
        break;

    case JOY_RIGHT:
        joystick_state.pressed[joy_button_right] = 1;
        break;

    default:
        break;
    }
    
    if (need_redraw)
    {
        display_playlist(plv, pl);
        need_redraw = 0;
    }
    if (joystick_state.process[joy_button_up] > 1)
    {
        joystick_state.process[joy_button_up] = 0;
        up(plv);
        need_redraw = 1;
    }
    if (joystick_state.process[joy_button_down] > 1)
    {
        joystick_state.process[joy_button_down] = 0;
        down(plv);
        need_redraw = 1;
    }
    if (joystick_state.process[joy_button_right] > 1)
    {
        joystick_state.process[joy_button_right] = 0;
        play(plv, pl);
        if (open_song(pl, &file))
        {
            BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
        }
        AudioFileSize = file.size;
        buffer_ctl.fptr = 0;
        need_redraw = 1;
    }
    
    /*
    switch (JoyState)
    {
    case JOY_UP:
      HAL_Delay(100);
      // Increase volume by 5% 
      if (uwVolume < 95)
        uwVolume += 5;
      else
        uwVolume = 100;
      sprintf((char*)FreqStr,"       VOL:    %lu     ",uwVolume);
      BSP_AUDIO_OUT_SetVolume(uwVolume);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 55, (uint8_t *)FreqStr, CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
      break;
      
    case JOY_DOWN:
      HAL_Delay(100);
      // Decrease volume by 5% 
      if (uwVolume > 5)
        uwVolume -= 5;
      else
        uwVolume = 0;
      sprintf((char*)FreqStr,"       VOL:    %lu     ",uwVolume);
      BSP_AUDIO_OUT_SetVolume(uwVolume);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 55, (uint8_t *)FreqStr, CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
      break;
      
    case JOY_LEFT:
      HAL_Delay(100);
      //Decrease Frequency 
      if (*AudioFreq_ptr != 8000)
      {
        AudioFreq_ptr--;
        BSP_AUDIO_OUT_SetFrequency(*AudioFreq_ptr);
      }
      sprintf((char*)FreqStr,"      FREQ: %lu     ", *AudioFreq_ptr);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 40, (uint8_t *)FreqStr, CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
      break;
      
    case JOY_RIGHT:
      HAL_Delay(100);
      // Increase Frequency 
      if (*AudioFreq_ptr != 96000)
      {
        AudioFreq_ptr++;
        BSP_AUDIO_OUT_SetFrequency(*AudioFreq_ptr);
      }
      sprintf((char*)FreqStr,"      FREQ: %lu     ",*AudioFreq_ptr);
      BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 40, (uint8_t *)FreqStr, CENTER_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
      break;
      
    case JOY_SEL:
      // Set Pause / Resume or Exit 
      HAL_Delay(200);
      if (BSP_JOY_GetState() == JOY_SEL)  // Long press on joystick selection button : Pause/Resume 
      {
        if (uwPauseEnabledStatus == 1)
        { // Pause is enabled, call Resume 
          BSP_AUDIO_OUT_Resume();
          uwPauseEnabledStatus = 0;
          BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       PLAYING...     ", CENTER_MODE);
        } 
        else
        { // Pause the playback 
          BSP_AUDIO_OUT_Pause();
          uwPauseEnabledStatus = 1;
          BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       PAUSE  ...     ", CENTER_MODE);
        }
        BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
        HAL_Delay(200);
      }
      else  // Short press on joystick selection button : exit 
      {
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        AUDIO_Stop();
        return;
      }
      break;
      
    default:
      break;
    }
    */
  }
}

/**
  * @brief  Display Audio demo hint
  * @param  None
  * @retval None
  */
static void Audio_SetHint(void)
{
  /* Clear the LCD */ 
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  
  /* Set Audio Demo description */
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), HEADBAND_HEIGHT);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE); 
  BSP_LCD_SetFont(&Font16);
  //BSP_LCD_DisplayStringAt(0, 0, (uint8_t *)"AUDIO PLAY", CENTER_MODE);
  /*
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, 20, (uint8_t *)"JOY_CLICK = exit / pause", LEFT_MODE);
  BSP_LCD_DisplayStringAt(0, 35, (uint8_t *)"JOY U/D   = change Volume", LEFT_MODE);
  BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)"JOY L/R   = change Freq", LEFT_MODE);
  */
}


/**
  * @brief  Starts Audio streaming.    
  * @param  audio_start_address : buffer start address
  * @param  audio_file_size : buffer size in bytes
  * @retval Audio error
  */ 
AUDIO_ErrorTypeDef AUDIO_Start ()
{
  uint32_t bytesread;


  //if (open(&file, file_name, file_name_len))
  if (open_song(pl, &file))
  {
    BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
    return AUDIO_ERROR_IO;
  }
  AudioFileSize = file.size;

  buffer_ctl.state = BUFFER_OFFSET_NONE;
  bytesread = get_all_data(&file,
                      &buffer_ctl.buff[0],
                      AUDIO_BUFFER_SIZE);
  if (bytesread > 0)
  {
    //BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE * sizeof(uint8_t) / sizeof(uint16_t));
    BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE);
    audio_state = AUDIO_STATE_PLAYING;      
    buffer_ctl.fptr = bytesread;
    return AUDIO_ERROR_NONE;
  }
  return AUDIO_ERROR_IO;
}

typedef struct tik_t
{
    uint16_t min;
    uint16_t sec;
    uint16_t ms;
} tik_t;

void byte_to_time (tik_t * time, uint32_t value)
{
    uint32_t divider = *AudioFreq_ptr;
    value = value / 2 / 2; // 16 bit (2 bytes)   2 channels
    time->min = (value / divider) / 60;
    time->sec = (value / divider) % 60;
    time->ms = (value % divider) * 1000 / divider;
}


/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
uint8_t AUDIO_Process(void)
{
  uint32_t bytesread;
  AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;  
  
  switch (audio_state)
  {
  case AUDIO_STATE_PLAYING:
    {
        tik_t cur_time;
        tik_t total_time;
  
        byte_to_time(&cur_time, buffer_ctl.fptr);
        byte_to_time(&total_time, AudioFileSize);

        char str[str_size];
        snprintf
        (
            str, 
            str_size, 
            " %4u:%02u.%03u / %u:%02u.%03u ", 
            cur_time.min,
            cur_time.sec,
            cur_time.ms,
            total_time.min,
            total_time.sec,
            total_time.ms
        );
        BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_SetFont(&Font12);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 180, (uint8_t *)str, CENTER_MODE);
    }

    if (buffer_ctl.fptr >= AudioFileSize)
    {
      /* Play audio sample again ... */
      buffer_ctl.fptr = 0; 
      next_playlist(pl);
      if (open_song(pl, &file))
      {
          BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
      }
      AudioFileSize = file.size;
      //f_seek(&file, 0); //TODO repeat mode
      error_state = AUDIO_ERROR_EOF;
      display_playlist(plv, pl);
    }

    /* 1st half buffer played; so fill it and continue playing from bottom*/
    if (buffer_ctl.state == BUFFER_OFFSET_HALF)
    {
      bytesread = get_all_data(&file,
                          &buffer_ctl.buff[0],
                          AUDIO_BUFFER_SIZE /2);
      
      if (bytesread > 0)
      { 
        buffer_ctl.state = BUFFER_OFFSET_NONE;
        buffer_ctl.fptr += bytesread; 
      }
    }
    
    /* 2nd half buffer played; so fill it and continue playing from top */    
    if (buffer_ctl.state == BUFFER_OFFSET_FULL)
    {
      bytesread = get_all_data(&file,
                          &buffer_ctl.buff[AUDIO_BUFFER_SIZE /2],
                          AUDIO_BUFFER_SIZE /2);
      if (bytesread > 0)
      {
        buffer_ctl.state = BUFFER_OFFSET_NONE;
        buffer_ctl.fptr += bytesread;
      }
    }
    break;
    
  default:
    error_state = AUDIO_ERROR_NOTREADY;
    break;
  }
  
  return (uint8_t) error_state;
}

AUDIO_ErrorTypeDef AUDIO_Stop(void)
{
    return AUDIO_ERROR_NONE;  
}

void reopen_file ()
{
}

/**
  * @brief  Gets Data from storage unit.
  * @param  None
  * @retval None
  */
/*
static uint32_t GetData(void *pdata, uint32_t offset, uint8_t *pbuf, uint32_t NbrOfData)
{
  uint8_t *lptr = pdata;
  uint32_t ReadDataNbr;
  
  ReadDataNbr = 0;
  while(((offset + ReadDataNbr) < AudioFileSize) && (ReadDataNbr < NbrOfData))
  {
    pbuf[ReadDataNbr]= lptr [offset + ReadDataNbr];
    ReadDataNbr++;
  }
  return ReadDataNbr;
}
*/
static uint32_t GetData (file_descriptor * _file, uint8_t * pbuf, uint32_t NbrOfData)
{
  //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 135, (uint8_t *)"       TRY READ", CENTER_MODE);
  uint32_t BytesRead = 0;
  uint32_t ret;
  while ((ret = f_read(_file, pbuf, NbrOfData /* * sizeof(uint8_t) / sizeof(BYTE)*/, (uint32_t *)&BytesRead)))
  {
    if (ret == eof_file)
    {
        return 0;
    }
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 120, (uint8_t *)"       ERR READ", CENTER_MODE);
    reopen_file();
  }
  return BytesRead /* * sizeof(BYTE) / sizeof(uint8_t)*/;
}

static inline uint32_t min (uint32_t a, uint32_t b)
{
    if (a < b)
        return a;
    else
        return b;
}

static uint32_t get_all_data (file_descriptor * _file, uint8_t *pbuf, uint32_t NbrOfData)
{
    uint32_t BytesRead = 0;
    while (BytesRead != NbrOfData)
    {
        uint32_t cnt_read = GetData(_file, pbuf + BytesRead, NbrOfData - BytesRead);
        if (cnt_read == 0)
        {
            return BytesRead;
        }
        BytesRead += cnt_read;
    }
    return BytesRead;
}

/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32412g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  if (audio_state == AUDIO_STATE_PLAYING)
  {
    /* allows AUDIO_Process() to refill 2nd part of the buffer  */
    buffer_ctl.state = BUFFER_OFFSET_FULL;
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
  if(audio_state == AUDIO_STATE_PLAYING)
  {
    /* allows AUDIO_Process() to refill 1st part of the buffer  */
    buffer_ctl.state = BUFFER_OFFSET_HALF;
  }
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Display message on the LCD screen */
  BSP_LCD_SetBackColor(LCD_COLOR_RED);
  BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"       DMA  ERROR     ", CENTER_MODE);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

  /* Stop the program with an infinite loop */
  while (BSP_PB_GetState(BUTTON_WAKEUP) != RESET)
  { return;}

  /* could also generate a system reset to recover from the error */
  /* .... */
}


  
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

