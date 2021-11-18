
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SAVE_FLASH_H
#define __SAVE_FLASH_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "esp_system.h"
#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 20  // Multisampling
  /* USER CODE BEGIN Includes */
  void save_flash(void);
  void load_flash(void);
  void nvsflash_init(void);
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
