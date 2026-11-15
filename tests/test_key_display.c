#include "unity.h"

#include "fake_key.h"
#include "fake_oled.h"
#include "key_display.h"

#include <string.h>

#define KEY_COUNT (KEY_ROWS * KEY_COLS)

void setUp(void) {
  memset(fake_key_pressed, 0, sizeof(fake_key_pressed));
  fake_oled_reset();
  key_display_init();
}

void tearDown(void) {}

static void assert_tiles(const bool expected_filled[KEY_COUNT]) {
  TEST_ASSERT_EQUAL_INT(KEY_COUNT, fake_oled_rect_count);
  for (int i = 0; i < KEY_COUNT; i++) {
    TEST_ASSERT_EQUAL(expected_filled[i], fake_oled_rects[i].filled);
  }
}

static void test_init_draws_outlined_tiles_on_screen(void) {
  bool expected[KEY_COUNT] = {false};
  TEST_ASSERT_EQUAL_INT(1, fake_oled_update_count);
  assert_tiles(expected);

  for (int i = 0; i < KEY_COUNT; i++) {
    FakeRect r = fake_oled_rects[i];
    TEST_ASSERT_TRUE(r.x >= 0 && r.x + r.w <= 128);
    TEST_ASSERT_TRUE(r.y >= 0 && r.y + r.h <= 64);
    if (i > 0)
      TEST_ASSERT_TRUE(r.x >= fake_oled_rects[i - 1].x + fake_oled_rects[i - 1].w);
  }
}

static void test_no_redraw_when_nothing_changes(void) {
  key_display_task();
  key_display_task();
  TEST_ASSERT_EQUAL_INT(1, fake_oled_update_count);
}

static void test_pressed_key_fills_only_its_tile(void) {
  fake_key_pressed[1] = true;
  key_display_task();

  bool expected[KEY_COUNT] = {false, true, false};
  TEST_ASSERT_EQUAL_INT(2, fake_oled_update_count);
  assert_tiles(expected);
}

static void test_held_key_stays_filled_without_redrawing(void) {
  fake_key_pressed[0] = true;
  key_display_task();
  for (int i = 0; i < 100; i++)
    key_display_task();

  bool expected[KEY_COUNT] = {true, false, false};
  TEST_ASSERT_EQUAL_INT(2, fake_oled_update_count);
  assert_tiles(expected);
}

static void test_release_returns_tile_to_outline(void) {
  fake_key_pressed[2] = true;
  key_display_task();
  fake_key_pressed[2] = false;
  key_display_task();

  bool expected[KEY_COUNT] = {false};
  TEST_ASSERT_EQUAL_INT(3, fake_oled_update_count);
  assert_tiles(expected);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_init_draws_outlined_tiles_on_screen);
  RUN_TEST(test_no_redraw_when_nothing_changes);
  RUN_TEST(test_pressed_key_fills_only_its_tile);
  RUN_TEST(test_held_key_stays_filled_without_redrawing);
  RUN_TEST(test_release_returns_tile_to_outline);
  return UNITY_END();
}
