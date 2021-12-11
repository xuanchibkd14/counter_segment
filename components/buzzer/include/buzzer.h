#include "esp_system.h"
#include "driver/gpio.h"
#define Buzzer_pin 26
#define Buzzer_pwm_en 0
// #define BUZZER_ON_LOW_LEVEL

#ifdef BUZZER_ON_LOW_LEVEL
#define BUZZER_ON gpio_set_level(Buzzer_pin, 0)
#define BUZZER_OFF gpio_set_level(Buzzer_pin, 1)
#else
#define BUZZER_ON gpio_set_level(Buzzer_pin, 1)
#define BUZZER_OFF gpio_set_level(Buzzer_pin, 0)
#endif
enum _buzzer_bip
{
    buzzer_off = 0,
    buzzer_short,
    buzzer_long,
    buzzer_long_repeat,
    buzzer_bip_num
};
typedef uint8_t buzzer_bip_t;

void buzzer_init(void);
void buzzer_task(void *arvgs);
void buzzer_set_bip(buzzer_bip_t buzzer);
