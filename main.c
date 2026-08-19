// Pico stuff
#include "pico/stdlib.h"
#include "tusb.h"

#include "key.h"
#include "logger.h"
#include "macropad_config.h"
#include "oled.h"

#include "ws2812.h"

typedef enum { MODE_KEYBOARD, MODE_CONFIG } DeviceMode;

DeviceMode get_device_mode(void) {
  gpio_pull_up(CONFIG_BTN_PIN);
  busy_wait_us(3); // settle time
  bool pressed = !gpio_get(CONFIG_BTN_PIN);
  return pressed ? MODE_CONFIG : MODE_KEYBOARD;
}

int main() {
  // Initialize Hardware
  oled_init(false);
  logger_init();
  key_init();
  ws2812_init();

  log_info("Macropad starting");

  DeviceMode mode = get_device_mode();

  if (mode == MODE_CONFIG) {
    // blue
    ws2812_set_rgb(0, 0, 255);
  } else {
    // white
    ws2812_set_rgb(255, 255, 255);
  }

  tusb_init();

  absolute_time_t next_scan = get_absolute_time();

  while (true) {
    tud_task();

    if (absolute_time_diff_us(get_absolute_time(), next_scan) <= 0) {
      next_scan = delayed_by_us(next_scan, 1000);
      key_scan();

      send_hid_report();
    }
  }
}
