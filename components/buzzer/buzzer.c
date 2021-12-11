#include <stdio.h>
#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static buzzer_bip_t buzzer_set_mode;
void buzzer_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    uint64_t i = 1;
    io_conf.pin_bit_mask = (i << Buzzer_pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);
    buzzer_set_bip(buzzer_short);
    xTaskCreate(buzzer_task, "buzzer", 2048, NULL, 8, NULL);
}
void buzzer_task(void *arvgs)
{
    while (1)
    {
        switch (buzzer_set_mode)
        {
        case buzzer_off:
            BUZZER_OFF;
            break;
        case buzzer_short:
            BUZZER_ON;
            printf("buzzer on short \n");
            vTaskDelay(200 / portTICK_RATE_MS);
            buzzer_set_mode = buzzer_off;
            break;
        case buzzer_long:
            BUZZER_ON;
            vTaskDelay(2000 / portTICK_RATE_MS);
            buzzer_set_mode = buzzer_off;
            break;
        case buzzer_long_repeat:
            BUZZER_ON;
            vTaskDelay(500 / portTICK_RATE_MS);
            BUZZER_OFF;
            vTaskDelay(500 / portTICK_RATE_MS);
            break;
        default:
            break;
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}
void buzzer_set_bip(buzzer_bip_t buzzer)
{
    buzzer_set_mode = buzzer;
}
