#include "unity.h"

#include "fake_tusb.h"
#include "hid_report.h"
#include "macropad_config.h"
#include "usb_descriptors.h"

#define SLOT_COUNT (KEY_ROWS * KEY_COLS)

void setUp(void) {
  // hid_report.c keeps static state: release everything and sync an empty
  // report so every test starts from "nothing pressed, nothing pending".
  fake_tusb_reset();
  for (uint8_t i = 0; i < SLOT_COUNT; i++) {
    hid_report_release(i);
  }
  hid_report_clear_macro();
  hid_report_task();
  fake_tusb_reset();
}
void tearDown(void) {}

static void assert_sent(uint8_t modifier, const uint8_t keys[6]) {
  TEST_ASSERT_EQUAL_UINT8(REPORT_ID_KEYBOARD, fake_report_id);
  TEST_ASSERT_EQUAL_HEX8(modifier, fake_report_modifier);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(keys, fake_report_keycodes, 6);
}

static void test_nothing_pending_at_rest(void) {
  TEST_ASSERT_FALSE(hid_report_pending());
  hid_report_task();
  TEST_ASSERT_EQUAL_INT(0, fake_report_count);
}

static void test_hold_sends_key_and_modifiers(void) {
  hid_report_hold(0, 0x04, 0x02);
  TEST_ASSERT_TRUE(hid_report_pending());

  hid_report_task();

  TEST_ASSERT_EQUAL_INT(1, fake_report_count);
  assert_sent(0x02, (const uint8_t[6]){0x04});
  TEST_ASSERT_FALSE(hid_report_pending());
}

static void test_unchanged_report_is_not_resent(void) {
  hid_report_hold(1, 0x05, 0);
  hid_report_task();
  hid_report_task();
  hid_report_hold(1, 0x05, 0); // same state again
  hid_report_task();
  TEST_ASSERT_EQUAL_INT(1, fake_report_count);
}

static void test_waits_until_host_is_ready(void) {
  fake_tud_hid_ready = false;
  hid_report_hold(0, 0x04, 0);

  hid_report_task();
  TEST_ASSERT_EQUAL_INT(0, fake_report_count);
  TEST_ASSERT_TRUE(hid_report_pending());

  fake_tud_hid_ready = true;
  hid_report_task();
  TEST_ASSERT_EQUAL_INT(1, fake_report_count);
  TEST_ASSERT_FALSE(hid_report_pending());
}

static void test_failed_send_is_retried(void) {
  fake_tud_hid_report_result = false;
  hid_report_hold(0, 0x04, 0);

  hid_report_task();
  TEST_ASSERT_EQUAL_INT(1, fake_report_count);
  TEST_ASSERT_TRUE(hid_report_pending());

  fake_tud_hid_report_result = true;
  hid_report_task();
  TEST_ASSERT_EQUAL_INT(2, fake_report_count);
  TEST_ASSERT_FALSE(hid_report_pending());
}

static void test_held_keys_and_macro_are_merged(void) {
  hid_report_hold(0, 0x04, 0x01);
  hid_report_hold(2, 0x06, 0x20);
  hid_report_set_macro(0x05, 0x04);
  hid_report_task();

  // Slots in order, then the macro stroke; modifiers OR'd
  assert_sent(0x01 | 0x20 | 0x04, (const uint8_t[6]){0x04, 0x06, 0x05});
}

static void test_duplicate_keycode_appears_once(void) {
  hid_report_hold(0, 0x04, 0);
  hid_report_hold(1, 0x04, 0x02);
  hid_report_set_macro(0x04, 0);
  hid_report_task();

  assert_sent(0x02, (const uint8_t[6]){0x04});
}

static void test_modifier_only_stroke_adds_no_keycode(void) {
  hid_report_hold(0, 0, 0x02); // e.g. a hold-mode Shift key
  hid_report_hold(1, 0x04, 0);
  hid_report_task();

  assert_sent(0x02, (const uint8_t[6]){0x04});
}

static void test_release_removes_only_that_slot(void) {
  hid_report_hold(0, 0x04, 0x01);
  hid_report_hold(1, 0x05, 0x02);
  hid_report_task();

  hid_report_release(0);
  TEST_ASSERT_TRUE(hid_report_pending());
  hid_report_task();

  TEST_ASSERT_EQUAL_INT(2, fake_report_count);
  assert_sent(0x02, (const uint8_t[6]){0x05});
}

static void test_releasing_everything_sends_empty_report(void) {
  hid_report_hold(0, 0x04, 0x01);
  hid_report_set_macro(0x05, 0);
  hid_report_task();

  hid_report_release(0);
  hid_report_clear_macro();
  hid_report_task();

  TEST_ASSERT_EQUAL_INT(2, fake_report_count);
  assert_sent(0, (const uint8_t[6]){0});
}

static void test_macro_stroke_replaces_previous_stroke(void) {
  hid_report_set_macro(0x04, 0x01);
  hid_report_task();
  hid_report_set_macro(0x05, 0);
  hid_report_task();

  assert_sent(0, (const uint8_t[6]){0x05});
}

static void test_same_key_pressed_again_is_not_pending(void) {
  // Pressing and releasing between two task calls cancels out: the macro
  // player relies on pending() to make sure each stroke reaches the host
  hid_report_set_macro(0x04, 0);
  hid_report_clear_macro();
  TEST_ASSERT_FALSE(hid_report_pending());
}

static void test_out_of_range_slot_is_ignored(void) {
  hid_report_hold(SLOT_COUNT, 0x04, 0x01);
  hid_report_hold(255, 0x05, 0x02);
  TEST_ASSERT_FALSE(hid_report_pending());

  hid_report_hold(0, 0x06, 0);
  hid_report_task();
  hid_report_release(SLOT_COUNT); // must not touch slot 0 or crash
  TEST_ASSERT_FALSE(hid_report_pending());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_nothing_pending_at_rest);
  RUN_TEST(test_hold_sends_key_and_modifiers);
  RUN_TEST(test_unchanged_report_is_not_resent);
  RUN_TEST(test_waits_until_host_is_ready);
  RUN_TEST(test_failed_send_is_retried);
  RUN_TEST(test_held_keys_and_macro_are_merged);
  RUN_TEST(test_duplicate_keycode_appears_once);
  RUN_TEST(test_modifier_only_stroke_adds_no_keycode);
  RUN_TEST(test_release_removes_only_that_slot);
  RUN_TEST(test_releasing_everything_sends_empty_report);
  RUN_TEST(test_macro_stroke_replaces_previous_stroke);
  RUN_TEST(test_same_key_pressed_again_is_not_pending);
  RUN_TEST(test_out_of_range_slot_is_ignored);
  return UNITY_END();
}
