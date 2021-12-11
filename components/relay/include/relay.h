#include "esp_system.h"
#include "driver/gpio.h"
#define relay_pin 12
#define relay_ON_LOW_LEVEL

#ifdef relay_ON_LOW_LEVEL
#define RELAY_ON gpio_set_level(relay_pin, 0)
#define RELAY_OFF gpio_set_level(relay_pin, 1)
#else
#define RELAY_ON gpio_set_level(relay_pin, 1)
#define RELAY_OFF gpio_set_level(relay_pin, 0)
#endif
enum _relay_num
{
    relay_0 = 0,
    relay_num
};
typedef uint8_t relay_t;

void relay_init(void);
void relay_set_state(relay_t relay, uint8_t relay_state);
