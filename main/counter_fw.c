/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "math.h"
#include "counter_fw.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "encoder_sw.h"
#include "timeout.h"
#include "button.h"
#include "hc595ic.h"
#include "save_flash.h"

counter_t counter_data = {
	.number = 0,
	.pointer = 500,
	.mode = 0,
	.reload = 1,
	.relay = 1,
	.buzzer = 1};

static void inputcounter_init(void);
static void led_blink_init(void);
static void shiftregister_task(void *args);
void IRAM_ATTR input_isr_handler(void *arg);
uint8_t segment_number[10] = {
	0x6f, // 0
	0x0A, // 1
	0x57, // 2
	0x1F, // 3
	0x3A, // 4
	0x3d, // 5
	0x7d, // 6
	0x0b, // 7
	0x7f, // 8
	0x3f, // 9
};
uint8_t segment_char[] = {
	0x7c, // b
	0x54, // c
	0x5E, // d
	0x58, // n
	0x60, // l
	0x50, // r
	0x18, // t
	0x4c, // u
	0x3A, // u
};
uint8_t segment_off = 0x00;
TO_TypeDef to_blink;

void app_main(void)
{
	hc595_init();

	BTN_Init();
	TO_Init();
	encoder_init();
	led_blink_init();
	inputcounter_init();
	nvsflash_init();
	if (counter_data.mode == 1 && counter_data.number == 0)
	{
		counter_data.number = counter_data.pointer;
	}

	TO_Start(&to_blink, 1000);
	xTaskCreate(shiftregister_task, "shift register task", 2048, NULL, 10, NULL);

	while (1)
	{
		gpio_set_level(gpio_led, !gpio_get_level(gpio_led));
		// printf("timout get cnt %d\r\n", TO_GetCurrentTime());
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
static void led_blink_init(void)
{
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	uint64_t i = 1;
	io_conf.pin_bit_mask = (i << gpio_led);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);
}
static void inputcounter_init(void)
{
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	uint64_t i = 1;
	io_conf.pin_bit_mask = (i << Input_cnt_pin);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	io_conf.pull_down_en = 0;
	gpio_config(&io_conf);
	gpio_isr_handler_add(Input_cnt_pin, input_isr_handler, (void *)Input_cnt_pin);
}
void IRAM_ATTR input_isr_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t)arg;
	if (gpio_get_level(gpio_num) == 0)
	{
		// counter up
		if (counter_data.mode == 0)
		{
			counter_data.number++;
			if (counter_data.number > counter_data.pointer)
			{
				counter_data.number = (counter_data.reload == 1) ? 1 : counter_data.pointer;
			}
		}
		else
		{
			counter_data.number--;
			if (counter_data.number == 0)
			{
				counter_data.number = (counter_data.reload == 1) ? counter_data.pointer : 0;
			}
		}
	}
}
uint8_t data_segment[Numofdigit];
static TO_TypeDef to_segblink;
static type_seg_t display_segment = digit_normal;

static bool led_stt = 1, leda = 1, ledb = 1, segblink;
static void (*btn_process)(void);
static void btn_normal(void);
static void btn_pointer(void);
static void btn_menu(void);
static void dipslay_menu(uint8_t menulist);
static void set_data_menu(uint8_t datamenu);
static void shiftregister_task(void *args)
{
	static uint8_t data_send[Numof595];
	static uint32_t temp = 0;

	TO_Start(&to_segblink, 500);
	btn_process = &btn_normal;
	while (1)
	{
		if (btn_process)
			btn_process();
		else
			btn_process = &btn_normal;

		dipslay_menu(display_segment);
		for (uint8_t i = 0; i < 5; i++)
		{
			data_send[0] = 1 << i | leda << led_a_bit | ledb << led_b_bit | led_stt << led_on_stt_bit;
			data_send[1] = data_segment[i];
			hc595_send_data(data_send, Numof595);
			vTaskDelay(1 / portTICK_RATE_MS);
		}
		// BTN_Clear();
	}
}
static void dipslay_menu(uint8_t menulist)
{
	switch (menulist)
	{
	case menu_mode_updown:
	{
		data_segment[0] = segment_char[seg_char_c];
		data_segment[1] = segment_char[seg_char_n];
		data_segment[2] = segment_char[seg_char_t];
		data_segment[3] = segment_off;
		counter_data.mode = encoder_get_cnt();
		data_segment[4] = (counter_data.mode == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_auto_reload:
	{
		data_segment[0] = segment_char[seg_char_r];
		data_segment[1] = segment_char[seg_char_l];
		data_segment[2] = segment_char[seg_char_d];
		data_segment[3] = segment_off;
		counter_data.reload = encoder_get_cnt();
		data_segment[4] = (counter_data.reload == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_buzzer:
	{
		data_segment[0] = segment_char[seg_char_b];
		data_segment[1] = segment_char[seg_char_u];
		data_segment[2] = segment_off;
		data_segment[3] = segment_off;
		counter_data.buzzer = encoder_get_cnt();
		data_segment[4] = (counter_data.buzzer == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_relay:
	{
		data_segment[0] = segment_char[seg_char_r];
		data_segment[1] = segment_char[seg_char_l];
		data_segment[2] = segment_char[seg_char_y];
		data_segment[3] = segment_off;
		counter_data.relay = encoder_get_cnt();
		data_segment[4] = (counter_data.relay == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case digit_overload:

	case digit_normal:
	{
		data_segment[4] = segment_number[counter_data.number % 10];
		data_segment[3] = segment_number[counter_data.number / 10 % 10];
		data_segment[2] = segment_number[counter_data.number / 100 % 10];
		data_segment[1] = segment_number[counter_data.number / 1000 % 10];
		data_segment[0] = segment_number[counter_data.number / 10000 % 10];
	}
	break;
	case digit_pointer:
	{
		counter_data.pointer = encoder_get_cnt();
		data_segment[4] = segment_number[counter_data.pointer % 10];
		data_segment[3] = segment_number[counter_data.pointer / 10 % 10];
		data_segment[2] = segment_number[counter_data.pointer / 100 % 10];
		data_segment[1] = segment_number[counter_data.pointer / 1000 % 10];
		data_segment[0] = segment_number[counter_data.pointer / 10000 % 10];
		if (segblink)
		{
			uint8_t i = (uint8_t)log10(encoder_get_gain());
			data_segment[4 - i] = 0;
		}
	}

	break;
	default:
		break;
	}
	if (segblink && menulist >= menu_mode_updown)
	{
		data_segment[4] = 0;
	}
}
static void btn_normal(void)
{
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		if (display_segment == digit_overload)
		{
			display_segment = digit_normal;
			printf("display normal \n");
		}
		else
		{
			printf("detect enc pressed but dont do anything\r\n");
		}
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
		encoder_set_cnt(counter_data.pointer);
		encoder_set_gain(1);
		encoder_set_range(0, 99999);
		display_segment = digit_pointer;
		btn_process = &btn_pointer;
		printf("detect 2 press go to set pointer \r\n");
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		display_segment = menu_mode_updown;
		set_data_menu(display_segment);
		encoder_set_gain(1);
		encoder_set_range(0, 1);
		btn_process = &btn_menu;
		printf("detect hold \t goto menu config \r\n");
	}
	else if (BTN_DetectRelease(Encoder_sw))
	{
		BTN_ClearRelease(Encoder_sw);
		printf("detect enc release \r\n");
	}
}
static void btn_pointer(void)
{
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		encoder_set_gain(10 * encoder_get_gain());
		if (encoder_get_gain() > 10000)
			encoder_set_gain(1);

		printf("detect press set gain %d \r\n", encoder_get_gain());
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		display_segment = digit_normal;
		btn_process = &btn_normal;
		save_flash();
		printf("detect hold \t goto normal \r\n");
	}
	else if (BTN_DetectRelease(Encoder_sw))
	{
		BTN_ClearRelease(Encoder_sw);
		printf("detect enc release \r\n");
	}
	if (TO_ReadStatus(&to_segblink))
	{
		TO_ClearStatus(&to_segblink);
		TO_Start(&to_segblink, 500);
		segblink = !segblink;
	}
	TO_Task(&to_segblink);
}

static void btn_menu(void)
{
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		display_segment = (display_segment == menu_relay) ? menu_mode_updown : display_segment + 1;
		set_data_menu(display_segment);
		printf("detect press go to menu %d \r\n", display_segment);
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		display_segment = digit_normal;
		btn_process = &btn_normal;
		save_flash();
		printf("detect hold \t goto digit normal \r\n");
	}
	else if (BTN_DetectRelease(Encoder_sw))
	{
		BTN_ClearRelease(Encoder_sw);
		printf("detect enc release \r\n");
	}
	if (TO_ReadStatus(&to_segblink))
	{
		TO_ClearStatus(&to_segblink);
		TO_Start(&to_segblink, 500);
		segblink = !segblink;
	}
	TO_Task(&to_segblink);
}
static void set_data_menu(uint8_t datamenu)
{
	switch (datamenu)
	{
	case menu_mode_updown:
		encoder_set_cnt(counter_data.mode);
		printf(" mode %d\n", counter_data.mode);
		break;
	case menu_auto_reload:
		encoder_set_cnt(counter_data.reload);
		printf(" reload %d\n", counter_data.reload);
		break;
	case menu_buzzer:
		encoder_set_cnt(counter_data.buzzer);
		printf(" buzzer %d\n", counter_data.buzzer);
		break;
	case menu_relay:
		encoder_set_cnt(counter_data.relay);
		printf(" relay %d\n", counter_data.relay);
		break;
	default:
		break;
	}
}