#include "pico/stdlib.h"
#include "tusb.h"

#include "config.h"
#include "hid_report.h"
#include "key.h"
#include "key_config.h"
#include "macropad_config.h"
#include "usb_descriptors.h"

static const uint8_t row_pins[KEY_ROWS] = KEY_ROW_PINS;
static const uint8_t col_pins[KEY_COLS] = KEY_COL_PINS;

// Debounced state (what the rest of the firmware sees)
static bool key_state[KEY_ROWS][KEY_COLS];
static bool prev_key_state[KEY_ROWS][KEY_COLS];

// Raw pin readings and when they last changed, used for debouncing
static bool raw_state[KEY_ROWS][KEY_COLS];
static absolute_time_t raw_changed_at[KEY_ROWS][KEY_COLS];
static absolute_time_t next_repeat[KEY_ROWS][KEY_COLS];

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
      raw_state[i][j] = false;
      raw_changed_at[i][j] = get_absolute_time();
    }
  }
}

// Returns the debounced state for key (r, c) given its latest raw reading.
// A new state is only accepted once the raw reading has been stable for
// KEY_DEBOUNCE_MS; any change in the raw reading restarts the timer.
static bool debounce_key(int r, int c, bool raw_pressed, absolute_time_t now) {
  if (raw_pressed != raw_state[r][c]) {
    raw_state[r][c] = raw_pressed;
    raw_changed_at[r][c] = now;
    return key_state[r][c];
  }

  int64_t stable_us = absolute_time_diff_us(raw_changed_at[r][c], now);
  bool is_stable = stable_us >= KEY_DEBOUNCE_US;

  return is_stable ? raw_pressed : key_state[r][c];
}

// send send 3.3V on rows, then check cols for any triggers
void key_scan() {
  absolute_time_t now = get_absolute_time();

  for (int i = 0; i < KEY_ROWS; i++) {
    // activate row
    gpio_put(row_pins[i], 0);
    busy_wait_us(3); // settle time

    for (int j = 0; j < KEY_COLS; j++) {
      bool raw_pressed = !gpio_get(col_pins[j]);
      key_state[i][j] = debounce_key(i, j, raw_pressed, now);
    }
    // deactivate row
    gpio_put(row_pins[i], 1);
  }
}

// Hold mode: mirror the physical key. Pressed on the down edge, released on
// the up edge; the host handles any key repeat.
static void handle_hold_key(int slot, const Macro *macro, bool pressed,
                            bool previously_pressed) {
  if (pressed && !previously_pressed) {
    const KeyStroke *stroke = &macro->strokes[0];
    hid_report_hold(slot, stroke->key, stroke->modifier);
  } else if (!pressed && previously_pressed) {
    hid_report_release(slot);
  }
}

// Sequence mode: play the macro once on press, then repeat while held.
static void handle_macro_key(int r, int c, const Macro *macro, bool pressed,
                             bool previously_pressed) {
  if (!pressed)
    return;

  absolute_time_t now = get_absolute_time();
  bool repeat_macro = absolute_time_diff_us(next_repeat[r][c], now) >= 0;

  if (!previously_pressed) {
    // First press: fire once, then wait before repeating
    macro_start(macro);
    next_repeat[r][c] = delayed_by_ms(now, MACRO_REPEAT_DELAY_MS);
  } else if (repeat_macro && macro_start(macro)) {
    // Held: repeat. If another macro is still running, retry next scan.
    next_repeat[r][c] = delayed_by_ms(now, MACRO_REPEAT_INTERVAL_MS);
  }
}

void process_key_events(void) {
  for (int r = 0; r < KEY_ROWS; r++) {
    for (int c = 0; c < KEY_COLS; c++) {
      int slot = r * KEY_COLS + c;
      const Macro *macro = &config.macros[slot];
      bool pressed = key_state[r][c];
      bool previously_pressed = prev_key_state[r][c];

      if (macro->mode == MACRO_MODE_HOLD && macro->length == 1) {
        handle_hold_key(slot, macro, pressed, previously_pressed);
      } else {
        handle_macro_key(r, c, macro, pressed, previously_pressed);
      }

      prev_key_state[r][c] = pressed;
    }
  }
}
