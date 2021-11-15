/**
 ******************************************************************************
 * @file    timeout.c
 * @author  DatLe
 * @version V1.00
 * @date    27-July-2013
 * @brief   STM32xx-EVAL abstraction layer.
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

/* Includes ------------------------------------------------------------------*/
#include "timeout.h"
#include "driver/timer.h"
#define TIMER_DIVIDER (8000)                           //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds
static uint32_t TO_SysTickCounter;
static void TO_SetStatus(TO_TypeDef *uTO);
static uint32_t timer_timeout;
/**
 * @brief : Initilize Time Out
 * @param : time out
 * @retval : none
 */
static bool IRAM_ATTR timer_group_isr_callback(void *args){
  TO_ISR();
  return 0;
}
void TO_Init(void)
{
  /* Select and initialize basic parameters of the timer */
  uint8_t timerindex =0;
  timer_config_t config = {
      .divider = TIMER_DIVIDER,
      .counter_dir = TIMER_COUNT_UP,
      .counter_en = TIMER_PAUSE,
      .alarm_en = TIMER_ALARM_EN,
      .auto_reload = true,
  }; // default clock source is APB
  timer_init(TIMER_GROUP_0, timerindex, &config);
  timer_set_counter_value(TIMER_GROUP_0, timerindex, 0);
   timer_set_alarm_value(TIMER_GROUP_0, timerindex, 10 );
  timer_enable_intr(TIMER_GROUP_0, timerindex);
  timer_isr_callback_add(TIMER_GROUP_0, timerindex, timer_group_isr_callback, (void *)timer_timeout, 0);
  timer_start(TIMER_GROUP_0, timerindex);
}


/**
 * @brief : Start the time out
 * @param : set time to time out occur
 *     @arg: setTime
 * @retval : none
 */
void TO_Start(TO_TypeDef *uTO, uint32_t setTime)
{
  uTO->Enable = 1;
  uTO->Time = setTime;
  uTO->CntTaget = setTime + TO_SysTickCounter;
}

/**
 * @brief : Stop the time out and clear it totally
 * @param : time out
 * @retval : none
 */
void TO_Stop(TO_TypeDef *uTO)
{
  uTO->Enable = 0;
  uTO->Time = 0;
  uTO->CntTaget = 0;
}

/**
 * @brief : return the time out status
 * @param : time out
 * @retval : time out status
 *     1: OCCUR
 *     0: NO_OCCUR
 */
TO_StatusTypeDef TO_ReadStatus(TO_TypeDef *uTO)
{
  return uTO->Status;
}

/**
 * @brief : Clear time out status and stop
 * @param : time out
 * @retval : none
 */
void TO_ClearStatus(TO_TypeDef *uTO)
{
  uTO->Status = TO_NOCCUR;
  TO_Stop(uTO);
}

/**
 * @brief : Set time out status
 * @param : time out
 * @retval : none
 */
static void TO_SetStatus(TO_TypeDef *uTO)
{
  uTO->Status = TO_OCCUR;
}

/**
 * @brief : Reset time out
 * @param : time out
 * @retval : none
 */
void TO_Reset(TO_TypeDef *uTO)
{
  uTO->Enable = 0;
  uTO->Time = 0;
  uTO->Status = TO_NOCCUR;
  uTO->CntTaget = 0;
}

/**
 * @brief : time out ISR handle, it is placed in Sys Tick ISR
 * @param : none
 * @retval : none
 */
void TO_ISR(void)
{
  TO_SysTickCounter++;
}

/**
 * @brief : Back ground time out task, check time out is occured or not?
 * @param : time out need to check
 * @retval : none
 */
void TO_Task(TO_TypeDef *uTO)
{
  if (uTO->Enable)
  {

    if (!((uTO->CntTaget - TO_SysTickCounter) <= uTO->Time))
    {
      TO_Stop(uTO);
      TO_SetStatus(uTO);
    }
  }
}

/**
 * @brief : Get current time
 * @param : none
 * @retval : current time
 */
uint32_t TO_GetCurrentTime(void)
{
  return TO_SysTickCounter;
}

/**
 * @}
 */

/**
 * @}
 */
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
