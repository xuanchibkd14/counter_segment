/**
 ******************************************************************************
 * File Name          : .c
 * Description        : This file provides code for the configuration
 *                      of the instances.
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

/* Includes ------------------------------------------------------------------*/
#include "encoder_sw.h"
#include "driver/gpio.h"
/* USER CODE BEGIN 0 */
void IRAM_ATTR encoder_isr_handler(void *arg);
encoder_t enc_data;
esp_err_t encoder_init(void)
{
    uint64_t i = 1;
    gpio_config_t io_conf;
    // interrupt of rising edge
    // io_conf.intr_type = GPIO_INTR_POSEDGE;

    io_conf.pin_bit_mask = i << encoder_pin_b;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    gpio_config_t io_int;
    io_int.intr_type = GPIO_INTR_POSEDGE;
    io_int.pin_bit_mask = (i << encoder_pin_a);
    // set as input mode
    io_int.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_int.pull_up_en = 1;
    gpio_config(&io_int);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(encoder_pin_a, encoder_isr_handler, (void *)encoder_pin_a);
    encoder_set_cnt(0);
    encoder_set_gain(1);
    encoder_set_range(0, 99999);
    return ESP_OK;
}
esp_err_t encoder_task(void)
{
    if (gpio_get_level(encoder_pin_a) == 1)
    {
        if (gpio_get_level(encoder_pin_b) == 0)
        {
            enc_data.encoder_counter = (enc_data.encoder_counter > enc_data.encoder_gain) ? enc_data.encoder_counter - enc_data.encoder_gain : enc_data.encoder_min;
        }
        else
        {
            enc_data.encoder_counter = (enc_data.encoder_counter > (enc_data.encoder_max - enc_data.encoder_gain)) ? enc_data.encoder_max : enc_data.encoder_counter + enc_data.encoder_gain;
        }
    }
    return ESP_OK;
}
uint32_t encoder_get_data(void)
{
    return enc_data.encoder_counter;
}
void IRAM_ATTR encoder_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == encoder_pin_a)
    {
        encoder_task();
    }
}
uint32_t encoder_get_cnt(void)
{
    return enc_data.encoder_counter;
}
void encoder_set_cnt(uint32_t num)
{
    enc_data.encoder_counter = num;
}
void encoder_set_gain(uint32_t gain)
{
    enc_data.encoder_gain = gain;
}
uint32_t encoder_get_gain(void)
{
    return enc_data.encoder_gain;
}
void encoder_set_range(uint32_t min, uint32_t max)
{
    enc_data.encoder_min = min;
    enc_data.encoder_max = max;
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
