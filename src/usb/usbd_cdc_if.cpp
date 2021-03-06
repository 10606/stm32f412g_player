/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "usbd_cdc_if.h"
#include "usb_command_process.h"
#include <algorithm>

#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048
volatile uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
volatile uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern "C"
{
extern volatile USBD_HandleTypeDef hUsbDeviceFS;

int8_t CDC_Init_FS(void);
int8_t CDC_DeInit_FS(void);
int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
int8_t CDC_Receive_FS(volatile uint8_t* pbuf, volatile uint32_t *Len);
int8_t CDC_TransmitCplt_FS(volatile uint8_t *pbuf, volatile uint32_t *Len, uint8_t epnum);


USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

int8_t CDC_Init_FS(void)
{
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 1);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

int8_t CDC_DeInit_FS(void)
{
  return (USBD_OK);
}

int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  static uint8_t tempbuf[7];
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:
        memcpy(tempbuf, pbuf, std::min(sizeof(tempbuf), static_cast <size_t> (length)));
    break;

    case CDC_GET_LINE_CODING:
        memcpy(pbuf, tempbuf, std::min(sizeof(tempbuf), static_cast <size_t> (length)));
    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
}
}

int8_t CDC_Receive_FS (volatile uint8_t * Buf, volatile uint32_t * Len)
{
    usb_process_v.receive_callback(Buf, *Len);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

uint8_t CDC_Transmit_FS(volatile uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (hcdc->TxState != 0)
    {
        return USBD_BUSY;
    }
    memcpy((uint8_t *)UserTxBufferFS, (uint8_t *)Buf, Len);
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    return result;
}

int8_t CDC_TransmitCplt_FS(volatile uint8_t *Buf, volatile uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  return result;
}

