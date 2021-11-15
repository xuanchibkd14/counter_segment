/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __HC595IC_H
#define __HC595IC_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "esp_system.h"
#define Numof595 2
#define HC595_DATA 16
#define HC595_CLK 5
#define HC595_LOAD 17
  /* USER CODE BEGIN Includes */
#define SEGMENT_ANODE_C1815

  esp_err_t hc595_send_data(uint8_t *senddata, uint8_t numdata);

  void hc595_init(void);
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
