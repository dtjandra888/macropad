#ifndef FAKE_GPIO_H
#define FAKE_GPIO_H

#include "hardware/gpio.h"

#define FAKE_GPIO_PINS 32

void fake_gpio_reset(void);

extern bool fake_gpio_initialized[FAKE_GPIO_PINS];
extern bool fake_gpio_is_output[FAKE_GPIO_PINS];
extern bool fake_gpio_pulled_up[FAKE_GPIO_PINS];
extern bool fake_gpio_out_level[FAKE_GPIO_PINS]; // last gpio_put value

// gpio_get() returns this hook's result if set, else true (pulled up)
extern bool (*fake_gpio_read_hook)(unsigned int pin);

#endif
