// Test stand-in for hardware/gpio.h: pin levels live in fakes/fake_gpio.c.
#ifndef HARDWARE_GPIO_H_STUB
#define HARDWARE_GPIO_H_STUB

#include <stdbool.h>
#include <stdint.h>

#define GPIO_IN false
#define GPIO_OUT true

void gpio_init(unsigned int pin);
void gpio_set_dir(unsigned int pin, bool out);
void gpio_put(unsigned int pin, bool value);
void gpio_pull_up(unsigned int pin);
bool gpio_get(unsigned int pin);

#endif
