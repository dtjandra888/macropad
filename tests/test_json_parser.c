#include "unity.h"

#include "json_parser.h"
#include "tusb.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static JsonParser parser_for(const char *text) {
  return (JsonParser){.data = text, .length = strlen(text), .position = 0};
}

// --- skip_whitespace / consume_char ---

static void test_skip_whitespace_skips_all_json_whitespace(void) {
  JsonParser p = parser_for(" \t\r\n x");
  skip_whitespace(&p);
  TEST_ASSERT_EQUAL_size_t(5, p.position);
}

static void test_skip_whitespace_stops_at_end(void) {
  JsonParser p = parser_for("   ");
  skip_whitespace(&p);
  TEST_ASSERT_EQUAL_size_t(3, p.position);
}

static void test_skip_whitespace_respects_length_not_nul(void) {
  // Only the first 2 bytes belong to the input
  JsonParser p = {.data = "  {", .length = 2, .position = 0};
  skip_whitespace(&p);
  TEST_ASSERT_EQUAL_size_t(2, p.position);
}

static void test_consume_char_after_whitespace(void) {
  JsonParser p = parser_for("  \n{");
  TEST_ASSERT_TRUE(consume_char(&p, '{'));
  TEST_ASSERT_EQUAL_size_t(4, p.position);
}

static void test_consume_char_mismatch_does_not_advance_past_it(void) {
  JsonParser p = parser_for(" [");
  TEST_ASSERT_FALSE(consume_char(&p, '{'));
  TEST_ASSERT_EQUAL_size_t(1, p.position);
}

static void test_consume_char_at_end_fails(void) {
  JsonParser p = parser_for("  ");
  TEST_ASSERT_FALSE(consume_char(&p, '}'));
}

// --- parse_string ---

static void test_parse_string_plain(void) {
  JsonParser p = parser_for("  \"HID_A\" rest");
  char buf[16];
  TEST_ASSERT_TRUE(parse_string(&p, buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("HID_A", buf);
  TEST_ASSERT_EQUAL_size_t(9, p.position);
}

static void test_parse_string_empty(void) {
  JsonParser p = parser_for("\"\"");
  char buf[4];
  TEST_ASSERT_TRUE(parse_string(&p, buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("", buf);
}

static void test_parse_string_escapes(void) {
  JsonParser p = parser_for("\"a\\\"b\\\\c\\/d\\be\\ff\\ng\\rh\\ti\"");
  char buf[32];
  TEST_ASSERT_TRUE(parse_string(&p, buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("a\"b\\c/d\be\ff\ng\rh\ti", buf);
}

static void test_parse_string_rejects_unicode_escape(void) {
  JsonParser p = parser_for("\"\\u0041\"");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_rejects_unknown_escape(void) {
  JsonParser p = parser_for("\"\\x\"");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_rejects_raw_control_char(void) {
  JsonParser p = parser_for("\"a\nb\"");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_rejects_unterminated(void) {
  JsonParser p = parser_for("\"abc");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_rejects_backslash_at_end(void) {
  JsonParser p = parser_for("\"abc\\");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_requires_opening_quote(void) {
  JsonParser p = parser_for("abc\"");
  char buf[16];
  TEST_ASSERT_FALSE(parse_string(&p, buf, sizeof(buf)));
}

static void test_parse_string_exact_fit(void) {
  // 3 chars + NUL fits a 4-byte buffer
  JsonParser p = parser_for("\"abc\"");
  char buf[4];
  TEST_ASSERT_TRUE(parse_string(&p, buf, sizeof(buf)));
  TEST_ASSERT_EQUAL_STRING("abc", buf);
}

static void test_parse_string_one_too_long(void) {
  // 4 chars + NUL does not fit a 4-byte buffer
  char buf[5] = {0, 0, 0, 0, 'X'};
  JsonParser p = parser_for("\"abcd\"");
  TEST_ASSERT_FALSE(parse_string(&p, buf, 4));
  TEST_ASSERT_EQUAL_CHAR('X', buf[4]); // nothing written past the buffer
}

// --- parse_modifiers ---

static void test_parse_modifiers_empty(void) {
  JsonParser p = parser_for(" [ ] ");
  uint8_t mods = 0xFF;
  TEST_ASSERT_TRUE(parse_modifiers(&p, &mods));
  TEST_ASSERT_EQUAL_HEX8(0, mods);
}

static void test_parse_modifiers_multiple_are_ored(void) {
  JsonParser p =
      parser_for("[ \"HID_MOD_LCTRL\" , \"HID_MOD_LSHIFT\",\"HID_MOD_RGUI\" ]");
  uint8_t mods = 0;
  TEST_ASSERT_TRUE(parse_modifiers(&p, &mods));
  TEST_ASSERT_EQUAL_HEX8(KEYBOARD_MODIFIER_LEFTCTRL |
                             KEYBOARD_MODIFIER_LEFTSHIFT |
                             KEYBOARD_MODIFIER_RIGHTGUI,
                         mods);
}

static void test_parse_modifiers_duplicate_is_harmless(void) {
  JsonParser p = parser_for("[\"HID_MOD_LALT\",\"HID_MOD_LALT\"]");
  uint8_t mods = 0;
  TEST_ASSERT_TRUE(parse_modifiers(&p, &mods));
  TEST_ASSERT_EQUAL_HEX8(KEYBOARD_MODIFIER_LEFTALT, mods);
}

static void test_parse_modifiers_rejects_unknown_name(void) {
  JsonParser p = parser_for("[\"HID_MOD_LCTRL\",\"HID_MOD_HYPER\"]");
  uint8_t mods = 0;
  TEST_ASSERT_FALSE(parse_modifiers(&p, &mods));
}

static void test_parse_modifiers_rejects_missing_close(void) {
  JsonParser p = parser_for("[\"HID_MOD_LCTRL\"");
  uint8_t mods = 0;
  TEST_ASSERT_FALSE(parse_modifiers(&p, &mods));
}

static void test_parse_modifiers_rejects_trailing_comma(void) {
  JsonParser p = parser_for("[\"HID_MOD_LCTRL\",]");
  uint8_t mods = 0;
  TEST_ASSERT_FALSE(parse_modifiers(&p, &mods));
}

static void test_parse_modifiers_rejects_non_array(void) {
  JsonParser p = parser_for("\"HID_MOD_LCTRL\"");
  uint8_t mods = 0;
  TEST_ASSERT_FALSE(parse_modifiers(&p, &mods));
}

static void test_parse_modifiers_rejects_missing_comma(void) {
  JsonParser p = parser_for("[\"HID_MOD_LCTRL\" \"HID_MOD_LALT\"]");
  uint8_t mods = 0;
  TEST_ASSERT_FALSE(parse_modifiers(&p, &mods));
}

// --- parse_stroke ---

static void test_parse_stroke_valid(void) {
  JsonParser p = parser_for(
      "{ \"key\": \"HID_C\", \"modifiers\": [\"HID_MOD_LCTRL\"] }");
  KeyStroke s = {0};
  TEST_ASSERT_TRUE(parse_stroke(&p, &s));
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_C, s.key);
  TEST_ASSERT_EQUAL_HEX8(KEYBOARD_MODIFIER_LEFTCTRL, s.modifier);
  TEST_ASSERT_EQUAL_size_t(p.length, p.position);
}

static void test_parse_stroke_modifier_only(void) {
  JsonParser p =
      parser_for("{\"key\":\"HID_NONE\",\"modifiers\":[\"HID_MOD_LSHIFT\"]}");
  KeyStroke s = {.key = 0xFF};
  TEST_ASSERT_TRUE(parse_stroke(&p, &s));
  TEST_ASSERT_EQUAL_HEX8(0, s.key);
  TEST_ASSERT_EQUAL_HEX8(KEYBOARD_MODIFIER_LEFTSHIFT, s.modifier);
}

static void test_parse_stroke_rejects_reversed_property_order(void) {
  JsonParser p = parser_for("{\"modifiers\":[],\"key\":\"HID_C\"}");
  KeyStroke s = {0};
  TEST_ASSERT_FALSE(parse_stroke(&p, &s));
}

static void test_parse_stroke_rejects_unknown_key(void) {
  JsonParser p = parser_for("{\"key\":\"HID_F24\",\"modifiers\":[]}");
  KeyStroke s = {0};
  TEST_ASSERT_FALSE(parse_stroke(&p, &s));
}

static void test_parse_stroke_rejects_missing_modifiers(void) {
  JsonParser p = parser_for("{\"key\":\"HID_C\"}");
  KeyStroke s = {0};
  TEST_ASSERT_FALSE(parse_stroke(&p, &s));
}

static void test_parse_stroke_rejects_extra_property(void) {
  JsonParser p =
      parser_for("{\"key\":\"HID_C\",\"modifiers\":[],\"extra\":1}");
  KeyStroke s = {0};
  TEST_ASSERT_FALSE(parse_stroke(&p, &s));
}

static void test_parse_stroke_rejects_long_property_name(void) {
  // Longer than the 16-byte property buffer
  JsonParser p = parser_for("{\"keyyyyyyyyyyyyyyyyyyy\":\"HID_C\"}");
  KeyStroke s = {0};
  TEST_ASSERT_FALSE(parse_stroke(&p, &s));
}

// --- parse_strokes ---

#define STROKE(k) "{\"key\":\"" k "\",\"modifiers\":[]}"

static void test_parse_strokes_empty(void) {
  JsonParser p = parser_for("[ ]");
  Macro m = {.length = 3};
  TEST_ASSERT_TRUE(parse_strokes(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(0, m.length);
}

static void test_parse_strokes_in_order(void) {
  JsonParser p =
      parser_for("[" STROKE("HID_H") ", " STROKE("HID_I") "]");
  Macro m = {0};
  TEST_ASSERT_TRUE(parse_strokes(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(2, m.length);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_H, m.strokes[0].key);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_I, m.strokes[1].key);
}

static void test_parse_strokes_max_allowed(void) {
  JsonParser p = parser_for("[" STROKE("HID_1") "," STROKE("HID_2") "," STROKE(
      "HID_3") "," STROKE("HID_4") "," STROKE("HID_5") "]");
  Macro m = {0};
  TEST_ASSERT_TRUE(parse_strokes(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MAX_STROKES, m.length);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_5, m.strokes[4].key);
}

static void test_parse_strokes_rejects_too_many(void) {
  JsonParser p = parser_for(
      "[" STROKE("HID_1") "," STROKE("HID_2") "," STROKE("HID_3") "," STROKE(
          "HID_4") "," STROKE("HID_5") "," STROKE("HID_6") "]");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_strokes(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MAX_STROKES, m.length); // never past the array
}

static void test_parse_strokes_rejects_trailing_comma(void) {
  JsonParser p = parser_for("[" STROKE("HID_A") ",]");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_strokes(&p, &m));
}

static void test_parse_strokes_rejects_unterminated(void) {
  JsonParser p = parser_for("[" STROKE("HID_A"));
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_strokes(&p, &m));
}

// --- parse_macro ---

static void test_parse_macro_defaults_to_sequence_mode(void) {
  JsonParser p = parser_for("{\"strokes\":[" STROKE("HID_A") "]}");
  Macro m = {.mode = MACRO_MODE_HOLD};
  TEST_ASSERT_TRUE(parse_macro(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_SEQUENCE, m.mode);
  TEST_ASSERT_EQUAL_UINT8(1, m.length);
}

static void test_parse_macro_explicit_macro_mode(void) {
  JsonParser p = parser_for(
      "{\"mode\":\"macro\",\"strokes\":[" STROKE("HID_A") "," STROKE("HID_B") "]}");
  Macro m = {0};
  TEST_ASSERT_TRUE(parse_macro(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_SEQUENCE, m.mode);
  TEST_ASSERT_EQUAL_UINT8(2, m.length);
}

static void test_parse_macro_hold_with_one_stroke(void) {
  JsonParser p =
      parser_for("{\"mode\":\"hold\",\"strokes\":[" STROKE("HID_SPACE") "]}");
  Macro m = {0};
  TEST_ASSERT_TRUE(parse_macro(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_HOLD, m.mode);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_SPACE, m.strokes[0].key);
}

static void test_parse_macro_properties_in_either_order(void) {
  JsonParser p = parser_for(
      " { \"strokes\" : [" STROKE("HID_SPACE") "] , \"mode\" : \"hold\" } ");
  Macro m = {0};
  TEST_ASSERT_TRUE(parse_macro(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_HOLD, m.mode);
  TEST_ASSERT_EQUAL_UINT8(1, m.length);
}

static void test_parse_macro_rejects_hold_with_two_strokes(void) {
  JsonParser p = parser_for("{\"mode\":\"hold\",\"strokes\":[" STROKE(
      "HID_A") "," STROKE("HID_B") "]}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_hold_with_no_strokes(void) {
  JsonParser p = parser_for("{\"mode\":\"hold\",\"strokes\":[]}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_allows_empty_sequence(void) {
  JsonParser p = parser_for("{\"strokes\":[]}");
  Macro m = {.length = 4};
  TEST_ASSERT_TRUE(parse_macro(&p, &m));
  TEST_ASSERT_EQUAL_UINT8(0, m.length);
}

static void test_parse_macro_rejects_missing_strokes(void) {
  JsonParser p = parser_for("{\"mode\":\"macro\"}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_unknown_property(void) {
  JsonParser p = parser_for("{\"strokes\":[],\"name\":\"x\"}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_unknown_mode(void) {
  JsonParser p = parser_for("{\"mode\":\"toggle\",\"strokes\":[]}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_long_mode_string(void) {
  // Longer than parse_mode's 8-byte buffer
  JsonParser p = parser_for("{\"mode\":\"holdholdhold\",\"strokes\":[]}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_trailing_comma(void) {
  JsonParser p = parser_for("{\"strokes\":[],}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_unterminated(void) {
  JsonParser p = parser_for("{\"strokes\":[]");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

static void test_parse_macro_rejects_empty_object(void) {
  JsonParser p = parser_for("{}");
  Macro m = {0};
  TEST_ASSERT_FALSE(parse_macro(&p, &m));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_skip_whitespace_skips_all_json_whitespace);
  RUN_TEST(test_skip_whitespace_stops_at_end);
  RUN_TEST(test_skip_whitespace_respects_length_not_nul);
  RUN_TEST(test_consume_char_after_whitespace);
  RUN_TEST(test_consume_char_mismatch_does_not_advance_past_it);
  RUN_TEST(test_consume_char_at_end_fails);

  RUN_TEST(test_parse_string_plain);
  RUN_TEST(test_parse_string_empty);
  RUN_TEST(test_parse_string_escapes);
  RUN_TEST(test_parse_string_rejects_unicode_escape);
  RUN_TEST(test_parse_string_rejects_unknown_escape);
  RUN_TEST(test_parse_string_rejects_raw_control_char);
  RUN_TEST(test_parse_string_rejects_unterminated);
  RUN_TEST(test_parse_string_rejects_backslash_at_end);
  RUN_TEST(test_parse_string_requires_opening_quote);
  RUN_TEST(test_parse_string_exact_fit);
  RUN_TEST(test_parse_string_one_too_long);

  RUN_TEST(test_parse_modifiers_empty);
  RUN_TEST(test_parse_modifiers_multiple_are_ored);
  RUN_TEST(test_parse_modifiers_duplicate_is_harmless);
  RUN_TEST(test_parse_modifiers_rejects_unknown_name);
  RUN_TEST(test_parse_modifiers_rejects_missing_close);
  RUN_TEST(test_parse_modifiers_rejects_trailing_comma);
  RUN_TEST(test_parse_modifiers_rejects_non_array);
  RUN_TEST(test_parse_modifiers_rejects_missing_comma);

  RUN_TEST(test_parse_stroke_valid);
  RUN_TEST(test_parse_stroke_modifier_only);
  RUN_TEST(test_parse_stroke_rejects_reversed_property_order);
  RUN_TEST(test_parse_stroke_rejects_unknown_key);
  RUN_TEST(test_parse_stroke_rejects_missing_modifiers);
  RUN_TEST(test_parse_stroke_rejects_extra_property);
  RUN_TEST(test_parse_stroke_rejects_long_property_name);

  RUN_TEST(test_parse_strokes_empty);
  RUN_TEST(test_parse_strokes_in_order);
  RUN_TEST(test_parse_strokes_max_allowed);
  RUN_TEST(test_parse_strokes_rejects_too_many);
  RUN_TEST(test_parse_strokes_rejects_trailing_comma);
  RUN_TEST(test_parse_strokes_rejects_unterminated);

  RUN_TEST(test_parse_macro_defaults_to_sequence_mode);
  RUN_TEST(test_parse_macro_explicit_macro_mode);
  RUN_TEST(test_parse_macro_hold_with_one_stroke);
  RUN_TEST(test_parse_macro_properties_in_either_order);
  RUN_TEST(test_parse_macro_rejects_hold_with_two_strokes);
  RUN_TEST(test_parse_macro_rejects_hold_with_no_strokes);
  RUN_TEST(test_parse_macro_allows_empty_sequence);
  RUN_TEST(test_parse_macro_rejects_missing_strokes);
  RUN_TEST(test_parse_macro_rejects_unknown_property);
  RUN_TEST(test_parse_macro_rejects_unknown_mode);
  RUN_TEST(test_parse_macro_rejects_long_mode_string);
  RUN_TEST(test_parse_macro_rejects_trailing_comma);
  RUN_TEST(test_parse_macro_rejects_unterminated);
  RUN_TEST(test_parse_macro_rejects_empty_object);
  return UNITY_END();
}
