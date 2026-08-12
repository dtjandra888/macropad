#include "pico/stdlib.h"
#include "tusb.h"

#include "key.h"
#include "logger.h"
#include "macropad_config.h"
#include "usb_descriptors.h"

static const uint8_t row_pins[KEY_ROWS] = KEY_ROW_PINS;
static const uint8_t col_pins[KEY_COLS] = KEY_COL_PINS;

static const uint8_t key_map[KEY_ROWS][KEY_COLS] = KEY_MAP;

static bool key_state[KEY_ROWS][KEY_COLS];

void key_init() {
  // Initialize GPIO
  // Notes for my non EE butt:
  // Row is signal source (GPIO_OUT); i.e. we control the voltage.
  // In key_scan, we set pin to 0 (i.e. stop sending 3.3v), then check if column
  // pin has anything. Column pins are GPIO_IN, meaning we want to read the
  // voltage off this. Pulldowns are weak resistors connected to ground to
  // prevent random external factors from triggering.
  for (int i = 0; i < KEY_ROWS; i++) {
    gpio_init(row_pins[i]);
    gpio_set_dir(row_pins[i], GPIO_OUT);
    gpio_put(row_pins[i], 1);
  }
  for (int i = 0; i < KEY_COLS; i++) {
    gpio_init(col_pins[i]);
    gpio_set_dir(col_pins[i], GPIO_IN);
    gpio_pull_up(col_pins[i]);
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
    gpio_put(row_pins[i], 0);
    busy_wait_us(3); // settle time

    for (int j = 0; j < KEY_COLS; j++) {
      bool pressed = !gpio_get(col_pins[j]);
      key_state[i][j]  =  pressed;
    }
    // deactivate row
    gpio_put(row_pins[i], 1);
  }
}

void send_hid_report(void) {
  if (!tud_hid_ready()) {
    return;
  }

  uint8_t keycode[6] = {0};
  int index = 0;

  for (int r = 0; r < KEY_ROWS; r++) {
    for (int c = 0; c < KEY_COLS; c++) {
      if (key_state[r][c] && index < 6) {
        keycode[index++] = key_map[r][c];
      }
    }
  }

  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
}
