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
#include "buzzer.h"
#include "relay.h"

counter_t counter_data = {
	.number = 0,
	.pointer = 500,
	.mode = 1,
	.reload = 1,
	.relay = 1,
	.buzzer = 1,
	.buzzer_alarm = 0};
uint8_t data_segment[Numofdigit];
uint8_t reset_counter_hand;
static TO_TypeDef to_segblink;
static type_seg_t display_segment = digit_normal;
void countertask(void);
void counteroverload(void);
static void set_start_cnt(void);
static void set_stop_cnt(void);
static void restart_cnt(void);
static bool led_stt = 1, ledred = 0, ledblue = 1, segblink;
static void (*btnprocess)(void);
static void (*counterprocess)(void);
static void btn_normal(void);
static void btn_pointer(void);
static void btn_menu(void);
static void btn_overload(void);
static void dipslay_menu(uint8_t menulist);
static void set_data_menu(uint8_t datamenu);
static void inputcounter_init(void);
static void led_blink_init(void);
static void shiftregister_task(void *args);
static void buzzer_bip(void);
void IRAM_ATTR input_isr_handler(void *arg);
static void relay_on(void);
static void relay_off(void);
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
	0x7b, // A
	0x7c, // b
	0x65, // c
	0x5E, // d
	0x75, // e
	0x71, // f
	0x64, // l
	0x58, // n
	0x5C, // o
	0x73, // p
	0x50, // r
	0x3d, // s
	0x61, // t
	0x4c, // u
	0x57, // z
};
uint8_t segment_off = 0x00;
static uint32_t counterfromISR = 0;
counter_state_t autoreload, enable_cnt = 0;
TO_TypeDef to_blink, to_led_run_stop, to_autoreset;
uint8_t resetbyinput = 0;
void app_main(void)
{
	hc595_init();

	BTN_Init();
	TO_Init();
	encoder_init();
	led_blink_init();
	inputcounter_init();
	nvsflash_init();
	buzzer_init();
	relay_init();
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
	static uint32_t to_check_noise = 0;
	if (gpio_get_level(gpio_num) == 0)
	{
		if (TO_GetCurrentTime() - to_check_noise > 2) // 0.2ms
		{
			to_check_noise = TO_GetCurrentTime();
			if (enable_cnt == counter_enable)
			{
				/* counter up */
				if (counter_data.mode == counter_up)
				{
					counter_data.number++;
					if (counter_data.number == counter_data.pointer)
					{
						relay_on();
						if (counter_data.reload == 0)
							TO_Start(&to_autoreset, 5000);
					}
					else if (counter_data.number > counter_data.pointer)
					{
						counter_data.number = counter_data.pointer;
						buzzer_bip();
						if (counter_data.reload == 0)
						{
							resetbyinput = 1;
							reset_counter();
						}
					}
				}
				else
				{
					counter_data.number--;
					if (counter_data.number == 0)
					{
						if (counter_data.reload == 0)
							TO_Start(&to_autoreset, 5000);
					}
					else if (counter_data.number < 0)
					{
						relay_on();
						counter_data.number = 0;
						buzzer_bip();
						if (counter_data.reload == 0)
						{
							resetbyinput = 1;
							reset_counter();
						}
					}
				}
			}
			if (counter_data.buzzer_alarm == 1)
			{
				buzzer_bip();
			}
		}
	}
}
static uint8_t cntled = 0;
uint8_t reset_cnt_by_cnt_mode;
static void shiftregister_task(void *args)
{
	static uint8_t data_send[Numof595];
	// static uint8_t led_blink_overload;
	static TO_TypeDef to_led_stt;
	TO_Start(&to_led_stt, 500);
	TO_Start(&to_segblink, 500);

	btnprocess = &btn_normal;
	counterprocess = &countertask;
	if (counter_data.number == counter_data.pointer || counter_data.number == 0)
	{
		counter_data.number = (counter_data.mode == 1) ? 0 : counter_data.pointer;
	}

	while (1)
	{
		if (btnprocess)
			btnprocess();
		else
			btnprocess = &btn_normal;
		if (counterprocess)
		{
			counterprocess();
		}
		else
			counterprocess = &countertask;
		dipslay_menu(display_segment);
		led_stt = (counter_data.mode) ? 0 : 1;
		for (uint8_t i = 0; i < 5; i++)
		{
			data_send[0] = 1 << i | ledred << led_red_bit | ledblue << led_blue_bit | led_stt << led_on_stt_bit;
			data_send[1] = data_segment[i];
			hc595_send_data(data_send, Numof595);
			vTaskDelay(1 / portTICK_RATE_MS);
		}
	}
}
static void dipslay_menu(uint8_t menulist)
{
	switch (menulist)
	{
	case menu_mode_updown:
	{
		counter_data.mode = encoder_get_cnt();
		if (reset_cnt_by_cnt_mode != counter_data.mode)
		{
			counter_data.number = (counter_data.mode == 1) ? 0 : counter_data.pointer;
		}

		data_segment[0] = segment_char[seg_char_c];
		data_segment[1] = segment_char[seg_char_t];
		data_segment[2] = segment_off;
		data_segment[3] = (counter_data.mode == 0) ? segment_char[seg_char_d] : segment_char[seg_char_u];
		data_segment[4] = (counter_data.mode == 0) ? segment_char[seg_char_o] : segment_char[seg_char_p];
	}
	break;
	case menu_auto_reload:
	{
		data_segment[0] = segment_char[seg_char_r];
		data_segment[1] = segment_char[seg_char_s];
		data_segment[2] = segment_char[seg_char_e];
		data_segment[3] = segment_char[seg_char_t];
		counter_data.reload = encoder_get_cnt();
		data_segment[4] = (counter_data.reload == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_buzzer:
	{
		data_segment[0] = segment_char[seg_char_b];
		data_segment[1] = segment_char[seg_char_u];
		data_segment[2] = segment_char[seg_char_z];
		data_segment[3] = segment_off;
		counter_data.buzzer = encoder_get_cnt();
		data_segment[4] = (counter_data.buzzer == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_buzzer_alarm:
	{
		data_segment[0] = segment_char[seg_char_b];
		data_segment[1] = segment_char[seg_char_u];
		data_segment[2] = segment_char[seg_char_a];
		data_segment[3] = segment_char[seg_char_l];
		;
		counter_data.buzzer_alarm = encoder_get_cnt();
		data_segment[4] = (counter_data.buzzer_alarm == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_relay:
	{
		data_segment[0] = segment_char[seg_char_r];
		data_segment[1] = segment_char[seg_char_e];
		data_segment[2] = segment_char[seg_char_l];
		data_segment[3] = segment_off;
		counter_data.relay = encoder_get_cnt();
		data_segment[4] = (counter_data.relay == 0) ? segment_number[0] : segment_number[1];
	}
	break;
	case menu_ofset:
		data_segment[0] = segment_char[seg_char_o];
		data_segment[1] = segment_char[seg_char_f];
		data_segment[2] = segment_char[seg_char_s];
		counter_data.ofset_counter = encoder_get_cnt();
		data_segment[3] = (counter_data.ofset_counter > 9) ? segment_number[counter_data.ofset_counter / 10] : segment_off;
		data_segment[4] = segment_number[counter_data.ofset_counter % 10];
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
		data_segment[4] = segment_off;
		if (menulist == menu_mode_updown || menulist == menu_ofset)
		{
			data_segment[3] = segment_off;
		}
	}
}
static void btn_normal(void)
{
	relay_off();
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		buzzer_bip();
		if (enable_cnt)
		{
			set_stop_cnt();
		}
		else
			set_start_cnt();
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
		set_stop_cnt();
		encoder_set_cnt(counter_data.pointer);
		buzzer_bip();
		encoder_set_gain(1);
		encoder_set_range(1, 99999);
		display_segment = digit_pointer;
		btnprocess = &btn_pointer;
		printf("detect 2 press go to set pointer \r\n");
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		set_stop_cnt();
		display_segment = menu_mode_updown;
		set_data_menu(display_segment);
		encoder_set_gain(1);
		encoder_set_range(0, 1);
		btnprocess = &btn_menu;
		printf("detect hold \t goto menu config \r\n");
		buzzer_bip();
	}
	else if (BTN_DetectRelease(Encoder_sw))
	{
		BTN_ClearRelease(Encoder_sw);
		printf("detect enc release \r\n");
	}
}
static void btn_pointer(void)
{
	enable_cnt = counter_disable;
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		encoder_set_gain(10 * encoder_get_gain());
		if (encoder_get_gain() > 10000)
			encoder_set_gain(1);

		printf("detect press set gain %d \r\n", encoder_get_gain());
		buzzer_bip();
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		display_segment = digit_normal;
		btnprocess = &btn_normal;
		set_stop_cnt();
		buzzer_bip();
		save_flash();
		counter_data.number = (counter_data.mode == 1) ? 0 : counter_data.pointer;
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
	enable_cnt = counter_disable;
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		display_segment = (display_segment == menu_ofset) ? menu_mode_updown : display_segment + 1;
		set_data_menu(display_segment);
		printf("detect press go to menu %d \r\n", display_segment);
		buzzer_bip();
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		display_segment = digit_normal;
		btnprocess = &btn_normal;
		save_flash();
		printf("detect hold \t goto digit normal \r\n");
		buzzer_bip();
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
	encoder_set_range(0, 1);
	switch (datamenu)
	{
	case menu_mode_updown:
		reset_cnt_by_cnt_mode = counter_data.mode;
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
	case menu_buzzer_alarm:
		encoder_set_cnt(counter_data.buzzer_alarm);
		printf("buzzer alarm %d\n", counter_data.buzzer_alarm);
		break;
	case menu_ofset:
		encoder_set_range(0, 99);
		encoder_set_cnt(counter_data.ofset_counter);
		printf("ofset counter %d\n", counter_data.ofset_counter);
		break;
	default:
		break;
	}
}
static void buzzer_bip(void)
{
	if (counter_data.buzzer == 1)
	{
		buzzer_set_bip(buzzer_short);
	}
}
static void relay_on(void)
{
	if (counter_data.relay == 0)
	{
		relay_set_state(0, 1);
	}
	else
	{
		relay_set_state(0, 0);
	}
}
static void relay_off(void)
{
	if (counter_data.relay == 0)
	{
		relay_set_state(0, 0);
	}
	else
	{
		relay_set_state(0, 1);
	}
}
static void set_start_cnt(void)
{
	enable_cnt = 1;
	ledred = led_off;
	ledblue = led_on;
}
static void set_stop_cnt(void)
{
	enable_cnt = 0;
	ledred = led_on;
	ledblue = led_off;
}
static void restart_cnt(void)
{
	autoreload = 0;
	counter_data.number = (counter_data.mode == 1) ? 0 : counter_data.pointer;
}
static uint8_t buzzer_overload;
void countertask(void)
{
	/* check menu/pointer setting */
	if (display_segment >= digit_pointer)
	{
		return;
	}
	/* checking counter to overload */
	static uint32_t warning_cnt;
	if (counter_data.ofset_counter < counter_data.pointer && counter_data.ofset_counter != 0)
	{
		warning_cnt = (counter_data.mode == counter_up) ? counter_data.pointer - counter_data.ofset_counter : counter_data.ofset_counter;
	}
	else
	{
		warning_cnt = (counter_data.mode == counter_up) ? counter_data.pointer : 0;
	}
	if (counter_data.mode == counter_up)
	{
		if (counter_data.number >= warning_cnt)
		{
			// printf("alarm prepair to overload \r\n");
			display_segment = digit_overload;
			counterprocess = &counteroverload;
			btnprocess = &btn_overload;
			buzzer_overload = 1;
		}
	}
	else
	{
		if (counter_data.number <= warning_cnt)
		{
			// printf("alarm prepair to overload \r\n");
			display_segment = digit_overload;
			counterprocess = &counteroverload;
			btnprocess = &btn_overload;
			buzzer_overload = 1;
		}
	}
}

void counteroverload(void)
{
	static TO_TypeDef _to_runtask;
	static uint8_t _notFirstTime = 0;
	if (_notFirstTime == 0)
	{
		_notFirstTime = 1;
		TO_Start(&_to_runtask, 100);
	}
	if (TO_ReadStatus(&_to_runtask))
	{
		TO_ClearStatus(&_to_runtask);
		TO_Start(&_to_runtask, 500);
		if (display_segment == digit_overload || buzzer_overload == 1)
		{
			buzzer_set_bip(buzzer_long_repeat);
			ledred = !ledred;
			ledblue = !ledblue;
		}
	}
	TO_Task(&_to_runtask);
	if (TO_ReadStatus(&to_autoreset))
	{
		TO_ClearStatus(&to_autoreset);
		reset_counter();
	}
	TO_Task(&to_autoreset);
}
static void btn_overload(void)
{
	if (BTN_DetectPress(Encoder_sw))
	{
		BTN_ClearPress(Encoder_sw);
		buzzer_overload = 0;
		if ((counter_data.mode == counter_up && counter_data.number >= counter_data.pointer) || (counter_data.mode == counter_down && counter_data.number <= 0))
		{
			reset_counter();
		}
		else
		{
			if (enable_cnt)
			{
				set_stop_cnt();
			}
			else
				set_start_cnt();
		}
		buzzer_bip();
	}
	else if (BTN_Detect2Press(Encoder_sw))
	{
		BTN_Clear2Press(Encoder_sw);
		set_stop_cnt();
		encoder_set_cnt(counter_data.pointer);
		buzzer_bip();
		encoder_set_gain(1);
		encoder_set_range(0, 99999);
		display_segment = digit_pointer;
		btnprocess = &btn_pointer;
		printf("detect 2 press go to set pointer \r\n");
	}
	else if (BTN_DetectHold(Encoder_sw))
	{
		BTN_ClearHold(Encoder_sw);
		set_stop_cnt();
		display_segment = menu_mode_updown;
		set_data_menu(display_segment);
		encoder_set_gain(1);
		encoder_set_range(0, 1);
		btnprocess = &btn_menu;
		printf("detect hold \t goto menu config \r\n");
		buzzer_bip();
	}
	else if (BTN_DetectRelease(Encoder_sw))
	{
		BTN_ClearRelease(Encoder_sw);
	}
}
void reset_counter(void)
{
	counter_data.number = (counter_data.mode == counter_up) ? 0 + resetbyinput : counter_data.pointer - resetbyinput;
	counterprocess = &countertask;
	btnprocess = &btn_normal;
	relay_off();
	buzzer_set_bip(buzzer_off);
	set_start_cnt();
	resetbyinput = 0;
}
/*End of line ---------------------------------------- copywriting by chi luong 2021 -----------------------------------*/