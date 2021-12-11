#include <stdio.h>
#include "relay.h"
void relay_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    uint64_t i = 1;
    io_conf.pin_bit_mask = (i << relay_pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);
    RELAY_OFF;
}

void relay_set_state(relay_t relay, uint8_t relay_state)
{
    if (relay_state == 0)
    {
        RELAY_OFF;
    }
    else
    {
        RELAY_ON;
    }
}
