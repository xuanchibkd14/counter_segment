/* Includes ------------------------------------------------------------------*/
#include "save_flash.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "counter_fw.h"
extern counter_t counter_data;
void nvsflash_init(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
void save_flash(void)
{
    esp_err_t err;
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");
        // Write
        printf("Updating restart counter in NVS ... ");
        uint64_t restart_counter = counter_data.number | (counter_data.point << 17) |
                                   (counter_data.mode << 35) | (counter_data.reload << 36) |
                                   (counter_data.buzzer << 37) | (counter_data.relay << 38);
        err = nvs_set_u64(my_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
}

/* USER CODE BEGIN 0 */
void load_flash(void)
{
    esp_err_t err;
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        uint64_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_u64(my_handle, "restart_counter", &restart_counter);

        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            // printf("Restart counter = %d\n", restart_counter);
            counter_data.number = restart_counter & 0x1ffff;
            counter_data.point = (restart_counter >> 17) & 0x1ffff;
            counter_data.mode = restart_counter >> 35;
            counter_data.buzzer = restart_counter >> 37;
            counter_data.reload = restart_counter >> 36;
            counter_data.relay = restart_counter >> 38;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            nvs_close(my_handle);
            save_flash();
            return;
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
