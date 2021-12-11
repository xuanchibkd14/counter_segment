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
#define Input_cnt_pin 4
#define gpio_led 2
#define Numofdigit 5
#define led_on_stt_bit 5
#define led_red_bit 6
#define led_blue_bit 7
#define led_on 0
#define led_off 1
  enum _type_segment
  {
    digit_normal = 0,
    digit_overload,
    digit_pointer,
    menu_mode_updown,
    menu_buzzer,
    menu_buzzer_alarm,
    menu_relay,
    menu_auto_reload,
    menu_ofset,
    typesegment_num
  };
  typedef uint8_t type_seg_t;
  typedef enum __counter_mode
  {
    counter_down = 0,
    counter_up
  } counter_mode_t;
  typedef enum __counter_state
  {
    counter_disable = 0,
    counter_enable
  } counter_state_t;
  enum _segment_character
  {
    seg_char_a = 0,
    seg_char_b,
    seg_char_c,
    seg_char_d,
    seg_char_e,
    seg_char_f,
    seg_char_l,
    seg_char_n,
    seg_char_o,
    seg_char_p,
    seg_char_r,
    seg_char_s,
    seg_char_t,
    seg_char_u,
    seg_char_z,
    seg_char_num
  };
  typedef uint8_t segment_char_t;
  /* USER CODE END Includes */
  typedef struct __counter_typedef
  {
    int32_t number;               // 17 counter 0->99999
    uint32_t pointer;             // 34 point to counter > 0-> 9999
    uint8_t ofset_counter;        // ofset to warning 0-99%
    counter_mode_t mode;          // 35 up/down
    counter_state_t reload;       // 36 auto reload
    counter_state_t buzzer;       // 37 on/off buzzer
    counter_state_t relay;        // 38 on/off relay
    counter_state_t buzzer_alarm; // alarm buzzer
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
