#include "pico/stdlib.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "key.h"
#include "logger.h"

#define KEY_LOGGING true

static const uint8_t row_pins[KEY_ROWS] = KEY_ROW_PINS;
static const uint8_t col_pins[KEY_COLS] = KEY_COL_PINS;

static bool key_state[KEY_ROWS][KEY_COLS];

void key_init() {
  // Initialize GPIO
  // Notes for my non EE butt:
  // Row is signal source (GPIO_OUT); i.e. we control the voltage
  // In key_scan, we set pin to 1 (i.e. send 3.3v), then check if column pin has
  // anything Column pins are GPIO_IN, meaning we want to read the voltage off
  // this Pulldowns are weak resistors connected to ground to prevent random
  // external factors from triggering
  for (int i = 0; i < KEY_ROWS; i++) {
    gpio_init(row_pins[i]);
    gpio_set_dir(row_pins[i], GPIO_OUT);
    gpio_put(row_pins[i], 0);
  }
  for (int i = 0; i < KEY_COLS; i++) {
    gpio_init(col_pins[i]);
    gpio_set_dir(col_pins[i], GPIO_IN);
    gpio_pull_down(col_pins[i]);
  }

  // set keys to not pressed
  for (int i = 0; i < KEY_ROWS; i++) {
    for (int j = 0; j < KEY_COLS; j++) {
      key_state[i][j] = false;
    }
  }
}

// send send 3.3V on rows, then check cols for any triggers
void key_scan() {
  for (int i = 0; i < KEY_ROWS; i++) {
    // activate row
    gpio_put(row_pins[i], 1);
    for (int j = 0; j < KEY_COLS; j++) {
      bool pressed = gpio_get(col_pins[j]);

      // Key pressed
      if (pressed && !key_state[i][j]) {
        key_state[i][j] = true;

        uint8_t keycode[6] = {HID_KEY_A}; // Sending all A's for now
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        if (KEY_LOGGING) {
          log_info("Key pressed");
        }
      }
      // key released
      else if (!pressed && key_state[i][j]) {
        key_state[i][j] = false;
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        if (KEY_LOGGING) {
          log_info("Key released");
        }
      }
    }
    // deactivate row
    gpio_put(row_pins[i], 0);
  }
}

#define KEY_PIN 2
void key_init_test(void) {
  gpio_init(KEY_PIN);
  gpio_set_dir(KEY_PIN, GPIO_IN);
  gpio_pull_down(KEY_PIN);

  log_info("Key initialized");
}

void key_scan_test(void) {
  static bool previous = false;

  bool current = gpio_get(KEY_PIN);

  if (current != previous) {
    previous = current;

    if (current) {
      uint8_t keycode[6] = {HID_KEY_A}; // Sending all A's for now
      tud_hid_keyboard_report(1, 0, keycode);
      log_info("KEY PRESSED");
    } else {
      tud_hid_keyboard_report(1, 0, NULL);
      log_info("KEY RELEASED");
    }
  }
}
