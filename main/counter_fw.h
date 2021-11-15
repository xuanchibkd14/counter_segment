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
#ifndef __COUNTER_FW_H
#define __COUNTER_FW_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
/* USER CODE BEGIN Includes */
#define Input_cnt_pin 26

  /* USER CODE END Includes */
  typedef struct __counter_typedef
  {
    uint64_t number : 17; // 17 counter 0->99999
    uint64_t point : 17;  // 34 point to counter > 0-> 9999
    uint64_t mode : 1;    // 35 up/down
    uint64_t reload : 1;  // 36 auto reload
    uint64_t buzzer : 1;  // 37 on/off buzzer
    uint64_t relay : 1;   // 38 on/off relay
  } counter_t;

  void inc_conter(void);
  void dec_counter(void);
  void reset_counter(void);
  /* USER CODE BEGIN Private defines */
  void IRAM_ATTR gpio_isr_handler(void *arg);
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
