#include "fake_gpio.h"

#include <string.h>

bool fake_gpio_initialized[FAKE_GPIO_PINS];
bool fake_gpio_is_output[FAKE_GPIO_PINS];
bool fake_gpio_pulled_up[FAKE_GPIO_PINS];
bool fake_gpio_out_level[FAKE_GPIO_PINS];
bool (*fake_gpio_read_hook)(unsigned int pin);

void fake_gpio_reset(void) {
  memset(fake_gpio_initialized, 0, sizeof(fake_gpio_initialized));
  memset(fake_gpio_is_output, 0, sizeof(fake_gpio_is_output));
  memset(fake_gpio_pulled_up, 0, sizeof(fake_gpio_pulled_up));
  memset(fake_gpio_out_level, 0, sizeof(fake_gpio_out_level));
  fake_gpio_read_hook = NULL;
}

void gpio_init(unsigned int pin) { fake_gpio_initialized[pin] = true; }
void gpio_set_dir(unsigned int pin, bool out) { fake_gpio_is_output[pin] = out; }
void gpio_put(unsigned int pin, bool value) { fake_gpio_out_level[pin] = value; }
void gpio_pull_up(unsigned int pin) { fake_gpio_pulled_up[pin] = true; }

bool gpio_get(unsigned int pin) {
  return fake_gpio_read_hook ? fake_gpio_read_hook(pin) : true;
}
