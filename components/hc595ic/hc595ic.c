#include <stdio.h>
#include "hc595ic.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "main/counter_fw.h"
// #include "timeout.h"
// #include "encoder_sw.h"


void hc595_init(void)
{
    printf("init hc595 \r\n");
    gpio_config_t hc595_gpio;
    hc595_gpio.intr_type = GPIO_INTR_DISABLE;
    hc595_gpio.mode = GPIO_MODE_OUTPUT;
    hc595_gpio.pin_bit_mask = (1 << HC595_DATA) | (1 << HC595_CLK) | (1 << HC595_LOAD);
    hc595_gpio.pull_down_en = 0;
    hc595_gpio.pull_up_en = 0;
    gpio_config(&hc595_gpio);
}

esp_err_t hc595_send_data(uint8_t *senddata, uint8_t numdata)
{
    if (numdata > Numof595)
    {
        printf("too much data can send \r\n");
    }
    gpio_set_level(HC595_LOAD, 0);
    gpio_set_level(HC595_DATA, 0);
    gpio_set_level(HC595_CLK, 0);
    for (uint8_t i = 0; i < numdata; i++)
    {
        uint8_t data_temp = senddata[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t _temp = data_temp & 0x80;
            gpio_set_level(HC595_CLK, 0);
            uint8_t a = (_temp == 0x80) ? 1 : 0;
            gpio_set_level(HC595_DATA, a);
            data_temp = data_temp << 1;
            gpio_set_level(HC595_CLK, 01);
        }
    }
    gpio_set_level(HC595_LOAD, 0);
    gpio_set_level(HC595_LOAD, 1);
    return ESP_OK;
}
// Given the number 12345 :
// 5 is 12345 % 10
// 4 is 12345 / 10 % 10
// 3 is 12345 / 100 % 10
// 2 is 12345 / 1000 % 10
// 1 is 12345 / 10000 % 10

// static void shiftregister_task(void *args)
// {
//     static uint8_t _notFirstTime = 0;
//     static bool led_blink;
//     if (_notFirstTime == 0)
//     {
//         _notFirstTime = 1;
//         TO_Start(&_to_runtask, 1);
//     }
//     while (1)
//     {
//         data_segment[4] = segment_number[counter_data.number % 10];
//         data_segment[3] = segment_number[counter_data.number / 10 % 10];
//         data_segment[2] = segment_number[counter_data.number / 100 % 10];
//         data_segment[1] = segment_number[counter_data.number / 1000 % 10];
//         data_segment[0] = segment_number[counter_data.number / 10000 % 10];
//         // data_segment[5] = segment_number[counter_data.number / 100000 % 10];
//         // blink led 1;
//         for (uint8_t i = 0; i < 5; i++)
//         {

//             data_send[0] = 1 << i;
//             data_send[1] = data_segment[i];
//             hc595_send_data(data_send, Numof595);
//             vTaskDelay(1 / portTICK_RATE_MS);
//         }
//     }
// }
