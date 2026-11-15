#include "unity.h"

#include "config.h"
#include "fake_gpio.h"
#include "fake_hid_report.h"
#include "fake_macro.h"
#include "fake_time.h"
#include "key.h"
#include "macropad_config.h"

Config config; // defined in config.c on the device

static const uint8_t row_pins[KEY_ROWS] = KEY_ROW_PINS;
static const uint8_t col_pins[KEY_COLS] = KEY_COL_PINS;

// Physical switch state. A column reads low only while its switch is closed
// and its row is driven low, like the real matrix.
static bool switch_closed[KEY_ROWS][KEY_COLS];

static bool matrix_read(unsigned int pin) {
  for (int c = 0; c < KEY_COLS; c++) {
    if (col_pins[c] != pin)
      continue;
    for (int r = 0; r < KEY_ROWS; r++) {
      bool row_driven = fake_gpio_is_output[row_pins[r]] &&
                        !fake_gpio_out_level[row_pins[r]];
      if (row_driven && switch_closed[r][c])
        return false;
    }
  }
  return true;
}

static void set_switch(int slot, bool closed) {
  switch_closed[slot / KEY_COLS][slot % KEY_COLS] = closed;
}

// One pass of the keyboard-mode main loop at the current time
static void scan(void) {
  key_scan();
  process_key_events();
}

// Advance time in 1 ms steps, scanning each step like main.c does
static void run_ms(int ms) {
  for (int i = 0; i < ms; i++) {
    fake_time_advance_ms(1);
    scan();
  }
}

// Close a switch and run until the press has been debounced
static void press_and_settle(int slot) {
  set_switch(slot, true);
  run_ms(KEY_DEBOUNCE_MS + 1);
}

static void release_and_settle(int slot) {
  set_switch(slot, false);
  run_ms(KEY_DEBOUNCE_MS + 1);
}

static Macro sequence_macro(uint8_t key) {
  return (Macro){.length = 1,
                 .mode = MACRO_MODE_SEQUENCE,
                 .strokes = {{key, 0}}};
}

static Macro hold_macro(uint8_t key, uint8_t modifiers) {
  return (Macro){.length = 1,
                 .mode = MACRO_MODE_HOLD,
                 .strokes = {{key, modifiers}}};
}

void setUp(void) {
  memset(switch_closed, 0, sizeof(switch_closed));
  memset(&config, 0, sizeof(config));
  config.macro_count = 3;
  config.macros[0] = sequence_macro(0x04);
  config.macros[1] = sequence_macro(0x05);
  config.macros[2] = sequence_macro(0x06);

  fake_gpio_reset();
  fake_gpio_read_hook = matrix_read;
  fake_time_set_us(10000000);

  // key.c keeps static state: re-init and settle with everything released
  key_init();
  scan();

  fake_hid_report_reset();
  fake_macro_reset();
}
void tearDown(void) {}

// --- key_init / key_scan ---

static void test_init_configures_matrix_pins(void) {
  for (int r = 0; r < KEY_ROWS; r++) {
    TEST_ASSERT_TRUE(fake_gpio_initialized[row_pins[r]]);
    TEST_ASSERT_TRUE(fake_gpio_is_output[row_pins[r]]);
    TEST_ASSERT_TRUE(fake_gpio_out_level[row_pins[r]]); // idle high
  }
  for (int c = 0; c < KEY_COLS; c++) {
    TEST_ASSERT_TRUE(fake_gpio_initialized[col_pins[c]]);
    TEST_ASSERT_FALSE(fake_gpio_is_output[col_pins[c]]);
    TEST_ASSERT_TRUE(fake_gpio_pulled_up[col_pins[c]]);
  }
}

static void test_scan_leaves_rows_idle_high(void) {
  set_switch(0, true);
  scan();
  for (int r = 0; r < KEY_ROWS; r++) {
    TEST_ASSERT_TRUE(fake_gpio_out_level[row_pins[r]]);
  }
}

static void test_nothing_happens_with_no_keys_pressed(void) {
  run_ms(1000);
  TEST_ASSERT_EQUAL_INT(0, fake_macro_start_count);
  TEST_ASSERT_EQUAL_INT(0, fake_hid_event_count);
}

// --- debounce ---

static void test_press_is_seen_after_debounce_time(void) {
  set_switch(0, true);
  scan(); // first scan that sees the change starts the timer

  fake_time_advance_us(KEY_DEBOUNCE_MS * 1000 - 1);
  scan();
  TEST_ASSERT_EQUAL_INT(0, fake_macro_start_count);

  fake_time_advance_us(1);
  scan();
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
}

static void test_key_is_pressed_reports_debounced_state(void) {
  TEST_ASSERT_FALSE(key_is_pressed(1));

  set_switch(1, true);
  scan();
  TEST_ASSERT_FALSE(key_is_pressed(1)); // still bouncing

  run_ms(KEY_DEBOUNCE_MS + 1);
  TEST_ASSERT_TRUE(key_is_pressed(1));
  TEST_ASSERT_FALSE(key_is_pressed(0));
  TEST_ASSERT_FALSE(key_is_pressed(-1));
  TEST_ASSERT_FALSE(key_is_pressed(KEY_ROWS * KEY_COLS));

  release_and_settle(1);
  TEST_ASSERT_FALSE(key_is_pressed(1));
}

static void test_bounce_restarts_debounce_timer(void) {
  set_switch(0, true);
  scan();
  run_ms(3);
  set_switch(0, false); // bounce
  run_ms(1);
  set_switch(0, true);
  run_ms(1); // timer restarts here

  run_ms(KEY_DEBOUNCE_MS - 1);
  TEST_ASSERT_EQUAL_INT(0, fake_macro_start_count);

  run_ms(1);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
}

static void test_short_glitch_is_ignored(void) {
  set_switch(0, true);
  run_ms(KEY_DEBOUNCE_MS - 1);
  set_switch(0, false);
  run_ms(100);
  TEST_ASSERT_EQUAL_INT(0, fake_macro_start_count);
}

static void test_release_is_debounced(void) {
  config.macros[1] = hold_macro(0x2C, 0);
  press_and_settle(1);
  fake_hid_report_reset();

  set_switch(1, false);
  scan();
  run_ms(KEY_DEBOUNCE_MS - 1);
  TEST_ASSERT_EQUAL_INT(0, fake_hid_event_count);

  run_ms(1);
  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);
  TEST_ASSERT_EQUAL_INT(HID_EVENT_RELEASE, fake_hid_events[0].type);
}

// --- hold mode ---

static void test_hold_key_held_and_released_with_switch(void) {
  config.macros[1] = hold_macro(0x2C, 0x02);

  press_and_settle(1);
  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);
  TEST_ASSERT_EQUAL_INT(HID_EVENT_HOLD, fake_hid_events[0].type);
  TEST_ASSERT_EQUAL_UINT8(1, fake_hid_events[0].slot);
  TEST_ASSERT_EQUAL_HEX8(0x2C, fake_hid_events[0].key);
  TEST_ASSERT_EQUAL_HEX8(0x02, fake_hid_events[0].modifiers);

  // Holding for a long time sends nothing more (the host repeats)
  run_ms(2000);
  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);

  release_and_settle(1);
  TEST_ASSERT_EQUAL_INT(2, fake_hid_event_count);
  TEST_ASSERT_EQUAL_INT(HID_EVENT_RELEASE, fake_hid_events[1].type);
  TEST_ASSERT_EQUAL_UINT8(1, fake_hid_events[1].slot);

  TEST_ASSERT_EQUAL_INT(0, fake_macro_start_count);
}

static void test_hold_mode_with_several_strokes_plays_as_sequence(void) {
  config.macros[0].mode = MACRO_MODE_HOLD;
  config.macros[0].length = 2;

  press_and_settle(0);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
  TEST_ASSERT_EQUAL_INT(0, fake_hid_event_count);
}

// --- sequence mode ---

static void test_sequence_key_starts_its_own_macro(void) {
  for (int slot = 0; slot < KEY_ROWS * KEY_COLS; slot++) {
    fake_macro_reset();
    press_and_settle(slot);
    TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
    TEST_ASSERT_EQUAL_PTR(&config.macros[slot], fake_macro_last_started);
    release_and_settle(slot);
  }
}

static void test_sequence_repeats_after_delay_then_interval(void) {
  set_switch(0, true);
  scan();
  fake_time_advance_ms(KEY_DEBOUNCE_MS);
  scan(); // first press: fires once
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);

  run_ms(MACRO_REPEAT_DELAY_MS - 1);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
  run_ms(1);
  TEST_ASSERT_EQUAL_INT(2, fake_macro_start_count);

  run_ms(MACRO_REPEAT_INTERVAL_MS - 1);
  TEST_ASSERT_EQUAL_INT(2, fake_macro_start_count);
  run_ms(1);
  TEST_ASSERT_EQUAL_INT(3, fake_macro_start_count);

  run_ms(MACRO_REPEAT_INTERVAL_MS * 10);
  TEST_ASSERT_EQUAL_INT(13, fake_macro_start_count);
}

static void test_short_press_fires_once(void) {
  press_and_settle(0);
  release_and_settle(0);
  run_ms(2000);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
}

static void test_repeat_retries_while_another_macro_runs(void) {
  set_switch(0, true);
  scan();
  fake_time_advance_ms(KEY_DEBOUNCE_MS);
  scan(); // first press
  run_ms(MACRO_REPEAT_DELAY_MS - 1);
  int calls_before = fake_macro_start_count;

  // Busy for 10 ms: every scan tries again
  fake_macro_start_result = false;
  run_ms(10);
  TEST_ASSERT_EQUAL_INT(calls_before + 10, fake_macro_start_count);

  // Accepted: the next repeat is a full interval later
  fake_macro_start_result = true;
  run_ms(1);
  int accepted_at = fake_macro_start_count;
  run_ms(MACRO_REPEAT_INTERVAL_MS - 1);
  TEST_ASSERT_EQUAL_INT(accepted_at, fake_macro_start_count);
  run_ms(1);
  TEST_ASSERT_EQUAL_INT(accepted_at + 1, fake_macro_start_count);
}

static void test_new_press_resets_repeat_delay(void) {
  press_and_settle(0);
  run_ms(MACRO_REPEAT_DELAY_MS + 200); // repeating
  release_and_settle(0);

  fake_macro_reset();
  press_and_settle(0);
  run_ms(MACRO_REPEAT_DELAY_MS - 10);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
}

static void test_hold_and_sequence_keys_together(void) {
  config.macros[2] = hold_macro(0x00, 0x02); // Shift

  set_switch(2, true);
  set_switch(0, true);
  run_ms(KEY_DEBOUNCE_MS + 1);

  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);
  TEST_ASSERT_EQUAL_INT(HID_EVENT_HOLD, fake_hid_events[0].type);
  TEST_ASSERT_EQUAL_UINT8(2, fake_hid_events[0].slot);
  TEST_ASSERT_EQUAL_INT(1, fake_macro_start_count);
  TEST_ASSERT_EQUAL_PTR(&config.macros[0], fake_macro_last_started);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_init_configures_matrix_pins);
  RUN_TEST(test_scan_leaves_rows_idle_high);
  RUN_TEST(test_nothing_happens_with_no_keys_pressed);
  RUN_TEST(test_press_is_seen_after_debounce_time);
  RUN_TEST(test_key_is_pressed_reports_debounced_state);
  RUN_TEST(test_bounce_restarts_debounce_timer);
  RUN_TEST(test_short_glitch_is_ignored);
  RUN_TEST(test_release_is_debounced);
  RUN_TEST(test_hold_key_held_and_released_with_switch);
  RUN_TEST(test_hold_mode_with_several_strokes_plays_as_sequence);
  RUN_TEST(test_sequence_key_starts_its_own_macro);
  RUN_TEST(test_sequence_repeats_after_delay_then_interval);
  RUN_TEST(test_short_press_fires_once);
  RUN_TEST(test_repeat_retries_while_another_macro_runs);
  RUN_TEST(test_new_press_resets_repeat_delay);
  RUN_TEST(test_hold_and_sequence_keys_together);
  return UNITY_END();
}
