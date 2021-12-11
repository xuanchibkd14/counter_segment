// save data when power off, detect via adc

/* Includes ------------------------------------------------------------------*/
#include "save_flash.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "counter_fw.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6; // GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_2;
static void adc_task(void *arg);
extern counter_t counter_data;
static float check_volt_input = 0;
float volt_input_get(void);
static TaskHandle_t adc_autosave;
static void adc_task(void *arg)
{
    // check_efuse();

    // Configure ADC
    adc2_config_channel_atten((adc2_channel_t)channel, atten);

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);

    // Continuously sample ADC1
    for (uint8_t i = 0; i < 200; i++)
    {
        check_volt_input += volt_input_get();
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    check_volt_input /= 200;
    if (check_volt_input < 3)
    {
        printf("debug mode \r\n");
        vTaskDelete(adc_autosave);
    }
    else
    {
        printf("volt input is %2.0f\r\n", check_volt_input);
    }
    while (1)
    {
        uint8_t real_voltage = volt_input_get();
        if (real_voltage < 7)
        {
            save_flash();
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
float volt_input_get(void)
{
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++)
    {
        int raw;
        adc2_get_raw((adc2_channel_t)channel, width, &raw);
        adc_reading += raw;
    }
    adc_reading /= NO_OF_SAMPLES;
    // Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    return ((float)voltage / 1000) / 0.09;
}
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
    load_flash();
    xTaskCreate(adc_task, "adc check power", 2048, NULL, 10, adc_autosave);
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
        printf("Updating counter in NVS ... ");
        uint32_t savecounter = counter_data.number;
        err = nvs_set_u32(my_handle, "savecounter", savecounter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        uint32_t savepointer = counter_data.pointer;
        err = nvs_set_u32(my_handle, "savepointer", savepointer);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        uint8_t saveofsset = counter_data.ofset_counter;
        err = nvs_set_u8(my_handle, "saveofset", saveofsset);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        uint8_t savemenu = counter_data.mode | counter_data.reload << 1 | counter_data.buzzer << 2 | counter_data.relay << 3 | counter_data.buzzer_alarm << 4;
        err = nvs_set_u8(my_handle, "savemenu", savemenu);
        printf((err != ESP_OK) ? "Failed!\n" : "Done 0x%x\n", savemenu);
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
        printf("Reading counter from NVS ... ");
        uint32_t savecounter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_u32(my_handle, "savecounter", &savecounter);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            counter_data.number = savecounter;
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
        uint32_t savepointer = 0;
        err = nvs_get_u32(my_handle, "savepointer", &savepointer);
        counter_data.pointer = savepointer;
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        uint8_t saveofset = 0;
        err = nvs_get_u8(my_handle, "saveofset", &saveofset);
        counter_data.ofset_counter = saveofset;
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        uint8_t savemenu = 0;
        err = nvs_get_u8(my_handle, "savemenu", &savemenu);
        counter_data.mode = savemenu & 1;
        counter_data.reload = (savemenu >> 1) & 1;
        counter_data.buzzer = (savemenu >> 2) & 1;
        counter_data.relay = (savemenu >> 3) & 1;
        counter_data.buzzer_alarm = (savemenu >> 4) & 1;
        printf((err != ESP_OK) ? "Failed! \n" : "Done 0x%x\n", savemenu);
        // Close
        nvs_close(my_handle);
    }

    printf("\n");
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
