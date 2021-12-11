/**
 ******************************************************************************
 * @file   :button.h
 * @author :DatLe
 * @version:v1_00
 * @date   :20/01/2016
 * @brief  :Header file for button.c module.
 ******************************************************************************
 ******************************************************************************
 */
/*
history
------------------------------------------------------------------------------
version  		author			date		      description
------------------------------------------------------------------------------
v1.00			  DatLe			  20/01/2016    initial build

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "esp_system.h"
#include "driver/gpio.h"

  /**
  @code

  @endcode
  */
  /* Select some of configs here!!!!!!!!! */
#define BUTTON_ACTIVE_LOW_LEVEL
  //#define BUTTON_ACTIVE_HIGH_LEVEL

#define BUTTON_MAX_BUFF 10
#define BUTTON_FILTER 2
#define BUTTON_HOLD_TIME_DF 1000
#define BUTTON_MULTI_PRESS_TIME_DF 300

#ifndef encoder_pin_sw
#define encoder_pin_sw 21
#endif
#ifndef boot_pin_sw
#define boot_pin_sw 0
#endif
  /* ----------------------------------------------------------------------- */

#define READ_BUTTON(__INDEX__) gpio_get_level(gpio_input_pin[__INDEX__])

  /**
   * @}
   */

  /** @defgroup Exported_Types
   * @{
   */

  typedef enum
  {
    Encoder_sw = 0,
    Boot_sw,
    BUTTON_NUM,
    BUTTON_NOPRESS
  } BTN_EnumTypeDef;

  /**
   * @brief  driver structure definition
   */

  void BTN_DeInit(void);
  void BTN_Handle(void);

  void BTN_ISR(void);

  bool BTN_DetectPress(BTN_EnumTypeDef Btn);
  bool BTN_Detect2Press(BTN_EnumTypeDef Btn);
  bool BTN_Detect3Press(BTN_EnumTypeDef Btn);
  bool BTN_DetectRelease(BTN_EnumTypeDef Btn);
  bool BTN_DetectHold(BTN_EnumTypeDef Btn);

  void BTN_ClearPress(BTN_EnumTypeDef Btn);
  void BTN_Clear2Press(BTN_EnumTypeDef Btn);
  void BTN_Clear3Press(BTN_EnumTypeDef Btn);
  void BTN_ClearRelease(BTN_EnumTypeDef Btn);
  void BTN_ClearHold(BTN_EnumTypeDef Btn);
  void BTN_Clear(void);
  /**
   * @}
   */

  /** @defgroup Exported_Constants
   * @{
   */

  /**
   * @}
   */

  /** @defgroup Exported_Macros
   * @{
   */
  /**
   * @}
   */

  /** @defgroup Exported_Functions
   * @{
   */
  /* Interface */
  void BTN_Init(void);
  /**
   * @}
   */

#ifdef __cplusplus
}
#endif

#endif /* ___H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2014 ACIS *****END OF FILE****/
