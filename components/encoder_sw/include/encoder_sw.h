/**
 ******************************************************************************
 * File Name          : RTC.h
 * Description        : This file provides code for the configuration
 *                      of the RTC instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENCODER_SW_H
#define __ENCODER_SW_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "esp_system.h"

/* USER CODE BEGIN Includes */
#define encoder_pin_a 19
#define encoder_pin_b 18
#define encoder_pin_sw 21
  typedef struct _enc_typedef
  {
    uint32_t encoder_counter;
    uint32_t encoder_sw_enabel;
  } encoder_t;

  esp_err_t encoder_init(void);
  esp_err_t encoder_task(void);
  uint32_t encoder_get_data(void);
  /* USER CODE END Includes */

  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

  /* USER CODE BEGIN Prototypes */

  /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
