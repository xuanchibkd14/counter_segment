/**
 ******************************************************************************
 * @file    :button.c
 * @author  :HuuChinh
 * @version :v1_00
 * @date    :18/02/2017
 * @brief   :
 *          This file should be added to the main application to use the provided
 *          functions that manage Leds, push-buttons, COM ports and low level
 *          HW resources initialization of the different modules available on
 *          STM32 evaluation boards from STMicroelectronics.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */
/*
  history
  ------------------------------------------------------------------------------
  version  		author			date		        description
  ------------------------------------------------------------------------------
  v1.00			  DatLe			  20/01/2016      initial build

  */
/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "timeout.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/** @addtogroup Utilities
 * @{
 */

/** @defgroup Abstraction_Layer
 * @{
 */

/** @defgroup Public_Variables
 * @{
 */
/** @defgroup Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @defgroup Private_Defines
 * @{
 */
typedef struct
{
  uint8_t MultiPressCnt;
  bool PressFlag;
  bool Press2Flag;
  bool Press3Flag;
  bool HoldFlag;
  bool ReleaseFlag;
  bool Press;
} Button_HandleTypeDef;

typedef struct
{
  Button_HandleTypeDef Button[BUTTON_NUM];
} Button_TypeDef;
/**
 * @}
 */

/** @defgroup Private_Macros
 * @{
 */

/**
 * @}
 */

/** @defgroup Private_Variables
 * @{
 */
static uint64_t gpio_input_pin[BUTTON_NUM] = {
    encoder_pin_sw};
Button_TypeDef ButtonProcess;
static bool _waitRelease[BUTTON_NUM];
static TO_TypeDef HoldTO[BUTTON_NUM];
static TO_TypeDef MultiPressTO[BUTTON_NUM];
/**
 * @}
 */

/** @defgroup FunctionPrototypes
 * @{
 */
/**
 * @}
 */
static void BTN_task(void *args)
{
  static TO_TypeDef _to_runtask;
  TO_Start(&_to_runtask, 10);
  while (1)
  {
    BTN_Handle();
    BTN_ISR();
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

/** @defgroup Private_Functions
 * @{
 */

/**
 * @brief : Init the button
 * @param : none
 * @retval : none
 */
void BTN_Init(void)
{
  gpio_config_t io_conf;
  // interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_DISABLE;
  uint64_t i = 1;
  io_conf.pin_bit_mask = (i << encoder_pin_sw);
  // set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  // enable pull-up mode
  io_conf.pull_up_en = 1;
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);
  xTaskCreate(BTN_task, "btn", 2048, NULL, 9, NULL);
}
/**
 * @brief : De init the button
 * @param : none
 * @retval : none
 */
/**
 * @brief : Button input task handle
 * @param : Get data from button buffer and process
 * @retval : return status of button(press, hold, double click)
 */
void BTN_Handle(void)
{
  uint8_t i;
  for (i = 0; i < BUTTON_NUM; i++)
  {
    if (TO_ReadStatus(&HoldTO[i]))
    {
      TO_ClearStatus(&HoldTO[i]);
      ButtonProcess.Button[i].HoldFlag = true;
    }
    TO_Task(&HoldTO[i]);

    if (TO_ReadStatus(&MultiPressTO[i]))
    {
      TO_ClearStatus(&MultiPressTO[i]);

      if (ButtonProcess.Button[i].MultiPressCnt == 1)
      {
        ButtonProcess.Button[i].PressFlag = true;
        if (ButtonProcess.Button[i].Press == false)
        {
          ButtonProcess.Button[i].ReleaseFlag = true;
        }
        else
        {
          _waitRelease[i] = true;
        }
      }
      else if (ButtonProcess.Button[i].MultiPressCnt == 2)
      {
        ButtonProcess.Button[i].Press2Flag = true;
      }
      else if (ButtonProcess.Button[i].MultiPressCnt == 3)
      {
        ButtonProcess.Button[i].Press3Flag = true;
      }
      ButtonProcess.Button[i].MultiPressCnt = 0;
    }
    TO_Task(&MultiPressTO[i]);
  }
}
/**
 * @brief : Checking Touch is pressed or not?
 * @param : none
 * @retval : true:Pressed, false: not press
 */
bool BTN_DetectPress(BTN_EnumTypeDef Btn)
{
  return ButtonProcess.Button[Btn].PressFlag;
}
bool BTN_Detect2Press(BTN_EnumTypeDef Btn)
{
  return ButtonProcess.Button[Btn].Press2Flag;
}
bool BTN_Detect3Press(BTN_EnumTypeDef Btn)
{
  return ButtonProcess.Button[Btn].Press3Flag;
}

/**
 * @brief : Checking Touch is released or not?
 * @param : none
 * @retval : true:Released, false: not release
 */
bool BTN_DetectRelease(BTN_EnumTypeDef Btn)
{
  return ButtonProcess.Button[Btn].ReleaseFlag;
}
bool BTN_DetectHold(BTN_EnumTypeDef Btn)
{
  return ButtonProcess.Button[Btn].HoldFlag;
}

/**
 * @brief : Checking Touch is double press or not?
 * @param : none
 * @retval : true:Double press, false: not double press
 */

/**
 * @brief : Checking Touch is hold or not?
 * @param : none
 * @retval : true:hold, false: not hold
 */

/**
 * @brief : Checking Touch is pressed or not?
 * @param : none
 * @retval : true:Pressed, false: not press
 */
void BTN_ClearPress(BTN_EnumTypeDef Btn)
{
  ButtonProcess.Button[Btn].PressFlag = false;
}
void BTN_Clear2Press(BTN_EnumTypeDef Btn)
{
  ButtonProcess.Button[Btn].Press2Flag = false;
}
void BTN_Clear3Press(BTN_EnumTypeDef Btn)
{
  ButtonProcess.Button[Btn].Press3Flag = false;
}

/**
 * @brief : Checking Touch is released or not?
 * @param : none
 * @retval : true:Released, false: not release
 */
void BTN_ClearRelease(BTN_EnumTypeDef Btn)
{
  ButtonProcess.Button[Btn].ReleaseFlag = false;
}
void BTN_ClearHold(BTN_EnumTypeDef Btn)
{
  ButtonProcess.Button[Btn].HoldFlag = false;
}

/**
 * @brief : Button_Buffer task in timer ISR
 * @param : none
 * @retval : none
 */
void BTN_ISR(void)
{
  volatile static uint8_t StatusPre[BUTTON_NUM];
  volatile static uint8_t Cnt[BUTTON_NUM];
  volatile uint8_t Status[BUTTON_NUM];
  volatile uint8_t i;

  for (i = 0; i < BUTTON_NUM; i++)
  {
#ifdef BUTTON_ACTIVE_LOW_LEVEL
    Status[i] = !READ_BUTTON(i);
#else
    Status[i] = READ_BUTTON(i);
#endif
    if (Status[i] == StatusPre[i])
    {
      Cnt[i]++;
      if (Cnt[i] == BUTTON_FILTER)
      {
        /* store which button is pressed */
        if (Status[i])
        {
          ButtonProcess.Button[i].Press = true;
          //          ButtonProcess.Button[i].PressFlag=true;
          TO_Start(&HoldTO[i], BUTTON_HOLD_TIME_DF);
          TO_Start(&MultiPressTO[i], BUTTON_MULTI_PRESS_TIME_DF);
          ButtonProcess.Button[i].MultiPressCnt++;
          _waitRelease[i] = false;
        }
        else
        {
          ButtonProcess.Button[i].Press = false;
          if (_waitRelease[i])
          {
            _waitRelease[i] = false;
            ButtonProcess.Button[i].ReleaseFlag = true;
          }
          TO_Stop(&HoldTO[i]);
        }
      };
      if (Cnt[i] > BUTTON_FILTER)
      {
        Cnt[i] = BUTTON_FILTER + 1;
      };
    }
    else
    {
      Cnt[i] = 0;
    };
    StatusPre[i] = Status[i];
  }
}

/**
 * @brief : BTN Clear all button flag
 * @param :
 * @retval :
 */
void BTN_Clear(void)
{
  uint8_t i;

  for (i = 0; i < BUTTON_NUM; i++)
  {
    ButtonProcess.Button[i].PressFlag = false;
    ButtonProcess.Button[i].Press2Flag = false;
    ButtonProcess.Button[i].Press3Flag = false;
    ButtonProcess.Button[i].ReleaseFlag = false;
    ButtonProcess.Button[i].HoldFlag = false;
  }
}
/**
 * @brief : Load default value for button config (HoldTime,doublePress TIme....)
 * @param : none
 * @retval : none
 */
/**
 * @brief  Configures Button GPIO.
 * @param  Button: Specifies the Button to be configured.
 *   This parameter can be one of following parameters:
 *     @arg none
 * @retval None
 */

/**
 * @brief  De init Button GPIO.
 * @param  Button: Specifies the Button to be Deinit.
 *   This parameter can be one of following parameters:
 *     @arg none
 * @retval None
 */
/******************* (C) COPYRIGHT 2015 ACIS *****END OF FILE****/
