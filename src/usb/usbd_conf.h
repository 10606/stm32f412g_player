/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_conf.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_conf.c file.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_pcd.h"
#include "stm32f4xx_ll_usb.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup USBD_OTG_DRIVER
  * @brief Driver for Usb device.
  * @{
  */

/** @defgroup USBD_CONF USBD_CONF
  * @brief Configuration file for Usb otg low level driver.
  * @{
  */

/** @defgroup USBD_CONF_Exported_Variables USBD_CONF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Defines USBD_CONF_Exported_Defines
  * @brief Defines for configuration of the Usb device.
  * @{
  */

/*---------- -----------*/
#define USBD_MAX_NUM_INTERFACES     1U
/*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION     1U
/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ     512U
/*---------- -----------*/
#define USBD_DEBUG_LEVEL     0U
/*---------- -----------*/
#define USBD_LPM_ENABLED     0U
/*---------- -----------*/
#define USBD_SELF_POWERED     0U

#define USB_OTGFS_OVRCR_Pin GPIO_PIN_7
#define USB_OTGFS_OVRCR_GPIO_Port GPIOG
#define USB_OTGFS_PPWR_EN_Pin GPIO_PIN_8
#define USB_OTGFS_PPWR_EN_GPIO_Port GPIOG
#define USB_OTGFS_VBUS_Pin GPIO_PIN_9
#define USB_OTGFS_VBUS_GPIO_Port GPIOA
#define USB_OTGFS_ID_Pin GPIO_PIN_10
#define USB_OTGFS_ID_GPIO_Port GPIOA
#define USB_OTGFS_DM_Pin GPIO_PIN_11
#define USB_OTGFS_DM_GPIO_Port GPIOA
#define USB_OTGFS_DP_Pin GPIO_PIN_12
#define USB_OTGFS_DP_GPIO_Port GPIOA

/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 		1
#define DEVICE_HS 		0

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Macros USBD_CONF_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* Memory management macros */

/** Alias for memory allocation. */
#define USBD_malloc         malloc

/** Alias for memory release. */
#define USBD_free           free

/** Alias for memory set. */
#define USBD_memset         memset

/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/** Alias for delay. */
#define USBD_Delay          HAL_Delay

/* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Types USBD_CONF_Exported_Types
  * @brief Types.
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_FunctionsPrototype USBD_CONF_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

/* Exported functions -------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
