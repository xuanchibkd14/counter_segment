/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "counter_fw.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "encoder_sw.h"
#include "timeout.h"
#include "button.h"
#include "hc595ic.h"
#include "save_flash.h"

const uint32_t ledblink = 2;
counter_t counter_data;
QueueHandle_t Queue_counter;
static void inputcounter_init(void);
static void led_blink_init(void);
TO_TypeDef to_blink;

void app_main(void)
{
	hc595_init();
	BTN_Init();
	TO_Init();
	// inputcounter_init();
	encoder_init();
	led_blink_init();
	nvsflash_init();
	load_flash();
	// save_flash();
	TO_Start(&to_blink, 1000);
	while (1)
	{
		static bool ledstate = 0;
		gpio_set_level(ledblink, ledstate);
		ledstate = !ledstate;
		// printf("timout get cnt %d\r\n", TO_GetCurrentTime());
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
static void inputcounter_init(void)
{
	gpio_config_t io_conf;
	// interrupt of rising edge
	io_conf.intr_type = GPIO_INTR_POSEDGE;
	uint64_t i = 1;
	io_conf.pin_bit_mask = (i << Input_cnt_pin);
	// set as input mode
	io_conf.mode = GPIO_MODE_INPUT;
	// enable pull-up mode
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);
	// gpio_set_intr_type(Input_cnt_pin, GPIO_INTR_ANYEDGE);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(Input_cnt_pin, gpio_isr_handler, (void *)Input_cnt_pin);
}
void IRAM_ATTR gpio_isr_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t)arg;
	if (gpio_num == encoder_pin_a)
	{
		encoder_task();
	}
	else if (gpio_num == Input_cnt_pin)
	{
		if (gpio_get_level(Input_cnt_pin) == 1)
		{
			counter_data.number++;
		}
	}
}
static void led_blink_init(void)
{
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	uint64_t i = 1;
	io_conf.pin_bit_mask = (i << ledblink);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);
}