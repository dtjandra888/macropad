#include "unity.h"

#include "fake_hid_report.h"
#include "fake_time.h"
#include "macro.h"

#define KEY_DELAY_MS 20 // MACRO_KEY_DELAY_MS in macro.c

static const Macro hello = {
    .length = 3,
    .strokes = {{0x0B, 0x02}, {0x08, 0}, {0x0F, 0}}, // H(shift) e l
};

void setUp(void) {
  // Finish any macro left running by the previous test
  fake_hid_report_reset();
  fake_time_set_us(1000000);
  for (int i = 0; i < 100 && macro_running(); i++) {
    fake_time_advance_ms(1000);
    macro_task();
  }
  TEST_ASSERT_FALSE(macro_running());
  fake_hid_report_reset();
}
void tearDown(void) {}

static void assert_event(int index, HidEventType type, uint8_t key,
                         uint8_t modifiers) {
  TEST_ASSERT_LESS_THAN_INT(fake_hid_event_count, index);
  TEST_ASSERT_EQUAL_INT(type, fake_hid_events[index].type);
  TEST_ASSERT_EQUAL_HEX8(key, fake_hid_events[index].key);
  TEST_ASSERT_EQUAL_HEX8(modifiers, fake_hid_events[index].modifiers);
}

static void test_start_rejects_null(void) {
  TEST_ASSERT_FALSE(macro_start(NULL));
  TEST_ASSERT_FALSE(macro_running());
}

static void test_start_rejects_empty_macro(void) {
  static const Macro empty = {.length = 0};
  TEST_ASSERT_FALSE(macro_start(&empty));
  TEST_ASSERT_FALSE(macro_running());
}

static void test_start_rejects_while_running(void) {
  static const Macro other = {.length = 1, .strokes = {{0x04, 0}}};
  TEST_ASSERT_TRUE(macro_start(&hello));
  TEST_ASSERT_TRUE(macro_running());
  TEST_ASSERT_FALSE(macro_start(&other));

  // The first macro is the one that plays
  macro_task();
  assert_event(0, HID_EVENT_SET_MACRO, 0x0B, 0x02);
}

static void test_task_does_nothing_when_idle(void) {
  macro_task();
  TEST_ASSERT_EQUAL_INT(0, fake_hid_event_count);
}

static void test_first_stroke_pressed_immediately(void) {
  macro_start(&hello);
  macro_task();

  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);
  assert_event(0, HID_EVENT_SET_MACRO, 0x0B, 0x02);
}

static void test_release_waits_for_key_delay(void) {
  macro_start(&hello);
  macro_task(); // press

  fake_time_advance_us(KEY_DELAY_MS * 1000 - 1);
  macro_task();
  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);

  fake_time_advance_us(1);
  macro_task();
  TEST_ASSERT_EQUAL_INT(2, fake_hid_event_count);
  assert_event(1, HID_EVENT_CLEAR_MACRO, 0, 0);
}

static void test_plays_all_strokes_in_order_then_stops(void) {
  macro_start(&hello);

  for (int i = 0; i < 20; i++) {
    macro_task();
    fake_time_advance_ms(KEY_DELAY_MS);
  }

  TEST_ASSERT_EQUAL_INT(6, fake_hid_event_count);
  assert_event(0, HID_EVENT_SET_MACRO, 0x0B, 0x02);
  assert_event(1, HID_EVENT_CLEAR_MACRO, 0, 0);
  assert_event(2, HID_EVENT_SET_MACRO, 0x08, 0);
  assert_event(3, HID_EVENT_CLEAR_MACRO, 0, 0);
  assert_event(4, HID_EVENT_SET_MACRO, 0x0F, 0);
  assert_event(5, HID_EVENT_CLEAR_MACRO, 0, 0);
  TEST_ASSERT_FALSE(macro_running());
}

static void test_next_press_waits_for_key_delay_after_release(void) {
  macro_start(&hello);
  macro_task(); // press 0
  fake_time_advance_ms(KEY_DELAY_MS);
  macro_task(); // release 0

  fake_time_advance_ms(KEY_DELAY_MS - 1);
  macro_task();
  TEST_ASSERT_EQUAL_INT(2, fake_hid_event_count);

  fake_time_advance_ms(1);
  macro_task();
  assert_event(2, HID_EVENT_SET_MACRO, 0x08, 0);
}

static void test_running_until_last_release(void) {
  static const Macro one = {.length = 1, .strokes = {{0x04, 0}}};
  macro_start(&one);
  macro_task(); // press
  TEST_ASSERT_TRUE(macro_running());

  fake_time_advance_ms(KEY_DELAY_MS);
  macro_task(); // release
  TEST_ASSERT_FALSE(macro_running());

  // A new macro can start right away
  TEST_ASSERT_TRUE(macro_start(&hello));
}

static void test_waits_while_report_pending(void) {
  macro_start(&hello);
  macro_task(); // press

  // The press hasn't reached the host yet: don't release, however long
  fake_hid_report_pending = true;
  fake_time_advance_ms(1000);
  macro_task();
  macro_task();
  TEST_ASSERT_EQUAL_INT(1, fake_hid_event_count);

  fake_hid_report_pending = false;
  macro_task();
  TEST_ASSERT_EQUAL_INT(2, fake_hid_event_count);
  assert_event(1, HID_EVENT_CLEAR_MACRO, 0, 0);
}

static void test_does_not_start_pressing_while_report_pending(void) {
  fake_hid_report_pending = true; // e.g. a held key change not sent yet
  macro_start(&hello);
  macro_task();
  TEST_ASSERT_EQUAL_INT(0, fake_hid_event_count);
}

static void test_repeats_same_key_with_release_between(void) {
  // "l" twice must reach the host as press/release/press/release
  static const Macro ll = {.length = 2, .strokes = {{0x0F, 0}, {0x0F, 0}}};
  macro_start(&ll);
  for (int i = 0; i < 10; i++) {
    macro_task();
    fake_time_advance_ms(KEY_DELAY_MS);
  }
  TEST_ASSERT_EQUAL_INT(4, fake_hid_event_count);
  assert_event(0, HID_EVENT_SET_MACRO, 0x0F, 0);
  assert_event(1, HID_EVENT_CLEAR_MACRO, 0, 0);
  assert_event(2, HID_EVENT_SET_MACRO, 0x0F, 0);
  assert_event(3, HID_EVENT_CLEAR_MACRO, 0, 0);
}

static void test_max_length_macro(void) {
  static const Macro five = {
      .length = MACRO_MAX_STROKES,
      .strokes = {{0x1E, 0}, {0x1F, 0}, {0x20, 0}, {0x21, 0}, {0x22, 0}}};
  macro_start(&five);
  for (int i = 0; i < 20; i++) {
    macro_task();
    fake_time_advance_ms(KEY_DELAY_MS);
  }
  TEST_ASSERT_EQUAL_INT(2 * MACRO_MAX_STROKES, fake_hid_event_count);
  assert_event(8, HID_EVENT_SET_MACRO, 0x22, 0);
  TEST_ASSERT_FALSE(macro_running());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_start_rejects_null);
  RUN_TEST(test_start_rejects_empty_macro);
  RUN_TEST(test_start_rejects_while_running);
  RUN_TEST(test_task_does_nothing_when_idle);
  RUN_TEST(test_first_stroke_pressed_immediately);
  RUN_TEST(test_release_waits_for_key_delay);
  RUN_TEST(test_plays_all_strokes_in_order_then_stops);
  RUN_TEST(test_next_press_waits_for_key_delay_after_release);
  RUN_TEST(test_running_until_last_release);
  RUN_TEST(test_waits_while_report_pending);
  RUN_TEST(test_does_not_start_pressing_while_report_pending);
  RUN_TEST(test_repeats_same_key_with_release_between);
  RUN_TEST(test_max_length_macro);
  return UNITY_END();
}
