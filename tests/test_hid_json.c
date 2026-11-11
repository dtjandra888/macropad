#include "unity.h"

#include "hid_json.h"
#include "tusb.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

typedef struct {
  uint8_t key;
  const char *name;
} KeyCase;

// Mirrors the table in hid_json.c: every entry must map both ways.
static const KeyCase keys[] = {
    {HID_KEY_NONE, "HID_NONE"},   {HID_KEY_A, "HID_A"},
    {HID_KEY_M, "HID_M"},         {HID_KEY_Z, "HID_Z"},
    {HID_KEY_1, "HID_1"},         {HID_KEY_9, "HID_9"},
    {HID_KEY_0, "HID_0"},         {HID_KEY_ENTER, "HID_ENTER"},
    {HID_KEY_ESCAPE, "HID_ESCAPE"}, {HID_KEY_BACKSPACE, "HID_BACKSPACE"},
    {HID_KEY_TAB, "HID_TAB"},     {HID_KEY_SPACE, "HID_SPACE"},
};

static void test_key_to_string_known_keys(void) {
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    TEST_ASSERT_EQUAL_STRING(keys[i].name, hid_key_to_string(keys[i].key));
  }
}

static void test_string_to_key_known_names(void) {
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    uint8_t key = 0xFF;
    TEST_ASSERT_TRUE_MESSAGE(string_to_hid_key(keys[i].name, &key),
                             keys[i].name);
    TEST_ASSERT_EQUAL_HEX8(keys[i].key, key);
  }
}

// Every letter and digit round-trips (catches a table typo)
static void test_all_letters_and_digits_round_trip(void) {
  for (uint8_t key = HID_KEY_A; key <= HID_KEY_0; key++) {
    const char *name = hid_key_to_string(key);
    TEST_ASSERT_NOT_NULL(name);

    uint8_t back = 0xFF;
    TEST_ASSERT_TRUE(string_to_hid_key(name, &back));
    TEST_ASSERT_EQUAL_HEX8(key, back);
  }
  TEST_ASSERT_EQUAL_STRING("HID_A", hid_key_to_string(HID_KEY_A));
  TEST_ASSERT_EQUAL_STRING("HID_Z", hid_key_to_string(HID_KEY_Z));
  TEST_ASSERT_EQUAL_STRING("HID_1", hid_key_to_string(HID_KEY_1));
}

static void test_unknown_key_returns_null(void) {
  TEST_ASSERT_NULL(hid_key_to_string(0xE0));
  TEST_ASSERT_NULL(hid_key_to_string(0xFF));
}

static void test_unknown_name_is_rejected_and_output_untouched(void) {
  uint8_t key = 0x55;
  TEST_ASSERT_FALSE(string_to_hid_key("HID_F13", &key));
  TEST_ASSERT_FALSE(string_to_hid_key("", &key));
  TEST_ASSERT_FALSE(string_to_hid_key("hid_a", &key));
  TEST_ASSERT_FALSE(string_to_hid_key("A", &key));
  TEST_ASSERT_EQUAL_HEX8(0x55, key);
}

static void test_string_to_modifier_all(void) {
  static const struct {
    const char *name;
    uint8_t mod;
  } mods[] = {
      {"HID_MOD_LCTRL", KEYBOARD_MODIFIER_LEFTCTRL},
      {"HID_MOD_LSHIFT", KEYBOARD_MODIFIER_LEFTSHIFT},
      {"HID_MOD_LALT", KEYBOARD_MODIFIER_LEFTALT},
      {"HID_MOD_LGUI", KEYBOARD_MODIFIER_LEFTGUI},
      {"HID_MOD_RCTRL", KEYBOARD_MODIFIER_RIGHTCTRL},
      {"HID_MOD_RSHIFT", KEYBOARD_MODIFIER_RIGHTSHIFT},
      {"HID_MOD_RALT", KEYBOARD_MODIFIER_RIGHTALT},
      {"HID_MOD_RGUI", KEYBOARD_MODIFIER_RIGHTGUI},
  };
  for (size_t i = 0; i < sizeof(mods) / sizeof(mods[0]); i++) {
    uint8_t mod = 0;
    TEST_ASSERT_TRUE_MESSAGE(string_to_modifier(mods[i].name, &mod),
                             mods[i].name);
    TEST_ASSERT_EQUAL_HEX8(mods[i].mod, mod);
  }
}

static void test_unknown_modifier_is_rejected(void) {
  uint8_t mod = 0x55;
  TEST_ASSERT_FALSE(string_to_modifier("HID_MOD_CTRL", &mod));
  TEST_ASSERT_FALSE(string_to_modifier("HID_A", &mod));
  TEST_ASSERT_EQUAL_HEX8(0x55, mod);
}

static void test_modifiers_to_json_empty(void) {
  char buf[64];
  size_t len = modifiers_to_json(buf, sizeof(buf), 0);
  TEST_ASSERT_EQUAL_STRING("[]", buf);
  TEST_ASSERT_EQUAL_size_t(2, len);
}

static void test_modifiers_to_json_single(void) {
  char buf[64];
  size_t len = modifiers_to_json(buf, sizeof(buf), KEYBOARD_MODIFIER_RIGHTALT);
  TEST_ASSERT_EQUAL_STRING("[\"HID_MOD_RALT\"]", buf);
  TEST_ASSERT_EQUAL_size_t(strlen(buf), len);
}

static void test_modifiers_to_json_all_in_table_order(void) {
  char buf[256];
  size_t len = modifiers_to_json(buf, sizeof(buf), 0xFF);
  TEST_ASSERT_EQUAL_STRING(
      "[\"HID_MOD_LCTRL\",\"HID_MOD_LSHIFT\",\"HID_MOD_LALT\","
      "\"HID_MOD_LGUI\",\"HID_MOD_RCTRL\",\"HID_MOD_RSHIFT\","
      "\"HID_MOD_RALT\",\"HID_MOD_RGUI\"]",
      buf);
  TEST_ASSERT_EQUAL_size_t(strlen(buf), len);
}

static void test_modifiers_to_json_never_writes_past_buffer(void) {
  char buf[32];
  memset(buf, 0xA5, sizeof(buf));
  size_t len = modifiers_to_json(buf, 10, 0xFF);
  TEST_ASSERT_EQUAL_size_t(9, len);
  TEST_ASSERT_EQUAL_STRING("[\"HID_MOD", buf);
  for (size_t i = 10; i < sizeof(buf); i++) {
    TEST_ASSERT_EQUAL_HEX8(0xA5, (uint8_t)buf[i]);
  }
}

static void test_json_append_tracks_offset(void) {
  char buf[16];
  size_t offset = 0;
  json_append(buf, sizeof(buf), &offset, "ab");
  json_append(buf, sizeof(buf), &offset, "%d", 42);
  TEST_ASSERT_EQUAL_STRING("ab42", buf);
  TEST_ASSERT_EQUAL_size_t(4, offset);
}

static void test_json_append_stops_when_full(void) {
  char buf[8];
  memset(buf, 0, sizeof(buf));
  size_t offset = 0;
  json_append(buf, 6, &offset, "abcdefgh");
  TEST_ASSERT_EQUAL_STRING("abcde", buf);
  TEST_ASSERT_EQUAL_size_t(5, offset);

  json_append(buf, 6, &offset, "zz"); // already full: no change
  TEST_ASSERT_EQUAL_STRING("abcde", buf);
  TEST_ASSERT_EQUAL_size_t(5, offset);
  TEST_ASSERT_EQUAL_CHAR(0, buf[6]);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_key_to_string_known_keys);
  RUN_TEST(test_string_to_key_known_names);
  RUN_TEST(test_all_letters_and_digits_round_trip);
  RUN_TEST(test_unknown_key_returns_null);
  RUN_TEST(test_unknown_name_is_rejected_and_output_untouched);
  RUN_TEST(test_string_to_modifier_all);
  RUN_TEST(test_unknown_modifier_is_rejected);
  RUN_TEST(test_modifiers_to_json_empty);
  RUN_TEST(test_modifiers_to_json_single);
  RUN_TEST(test_modifiers_to_json_all_in_table_order);
  RUN_TEST(test_modifiers_to_json_never_writes_past_buffer);
  RUN_TEST(test_json_append_tracks_offset);
  RUN_TEST(test_json_append_stops_when_full);
  return UNITY_END();
}
