#include "unity.h"

#include "config.h"
#include "config_api.h"
#include "fake_flash.h"
#include "fake_lwip.h"
#include "tusb.h"

#include <string.h>

// lwIP httpd POST hooks implemented by config_api.c
void httpd_post_begin(void *connection, const char *uri,
                      const char *http_request, u16_t http_request_len,
                      int content_len, char *response_uri,
                      u8_t response_uri_len, u8_t *post_auto_wnd);
err_t httpd_post_receive_data(void *connection, struct pbuf *p);
void httpd_post_finished(void *connection, char *response_uri,
                         u8_t response_uri_len);

// Size of the POST body buffer in config_api.c
#define POST_BUFFER_SIZE 2048

#define STROKE(k, mods) "{\"key\":\"" k "\",\"modifiers\":[" mods "]}"

static const char *VALID_JSON =
    "{ \"macros\": [\n"
    "  {\"mode\":\"macro\",\"strokes\":[" STROKE(
        "HID_C", "\"HID_MOD_LCTRL\",\"HID_MOD_LSHIFT\"") "]},\n"
    "  {\"mode\":\"hold\",\"strokes\":[" STROKE("HID_NONE",
                                               "\"HID_MOD_LSHIFT\"") "]},\n"
    "  {\"strokes\":[" STROKE("HID_H", "") "," STROKE("HID_I", "") "]}\n"
    "] }\n";

static Config empty_config(void) {
  Config c;
  memset(&c, 0, sizeof(c));
  return c;
}

void setUp(void) {
  fake_flash_reset();
  fake_lwip_reset();
  config = empty_config();
}
void tearDown(void) {}

// --- config_from_json ---

static void test_from_json_valid_document(void) {
  Config out = empty_config();
  TEST_ASSERT_TRUE(config_from_json(VALID_JSON, strlen(VALID_JSON), &out));

  TEST_ASSERT_EQUAL_UINT16(3, out.macro_count);

  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_SEQUENCE, out.macros[0].mode);
  TEST_ASSERT_EQUAL_UINT8(1, out.macros[0].length);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_C, out.macros[0].strokes[0].key);
  TEST_ASSERT_EQUAL_HEX8(
      KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT,
      out.macros[0].strokes[0].modifier);

  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_HOLD, out.macros[1].mode);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_NONE, out.macros[1].strokes[0].key);
  TEST_ASSERT_EQUAL_HEX8(KEYBOARD_MODIFIER_LEFTSHIFT,
                         out.macros[1].strokes[0].modifier);

  TEST_ASSERT_EQUAL_UINT8(MACRO_MODE_SEQUENCE, out.macros[2].mode);
  TEST_ASSERT_EQUAL_UINT8(2, out.macros[2].length);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_H, out.macros[2].strokes[0].key);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_I, out.macros[2].strokes[1].key);
}

static void test_from_json_empty_macros(void) {
  const char *json = "{\"macros\":[]}";
  Config out = empty_config();
  out.macro_count = 2;
  TEST_ASSERT_TRUE(config_from_json(json, strlen(json), &out));
  TEST_ASSERT_EQUAL_UINT16(0, out.macro_count);
}

static void test_from_json_rejects_more_macros_than_keys(void) {
  const char *json = "{\"macros\":[{\"strokes\":[]},{\"strokes\":[]},"
                     "{\"strokes\":[]},{\"strokes\":[]}]}";
  Config out = empty_config();
  TEST_ASSERT_FALSE(config_from_json(json, strlen(json), &out));
}

static void test_from_json_rejects_trailing_garbage(void) {
  const char *json = "{\"macros\":[]} x";
  Config out = empty_config();
  TEST_ASSERT_FALSE(config_from_json(json, strlen(json), &out));
}

static void test_from_json_allows_trailing_whitespace(void) {
  const char *json = "{\"macros\":[]} \r\n";
  Config out = empty_config();
  TEST_ASSERT_TRUE(config_from_json(json, strlen(json), &out));
}

static void test_from_json_rejects_wrong_top_level_property(void) {
  const char *json = "{\"keys\":[]}";
  Config out = empty_config();
  TEST_ASSERT_FALSE(config_from_json(json, strlen(json), &out));
}

static void test_from_json_rejects_unterminated(void) {
  const char *json = "{\"macros\":[{\"strokes\":[]}";
  Config out = empty_config();
  TEST_ASSERT_FALSE(config_from_json(json, strlen(json), &out));
}

static void test_from_json_rejects_null_arguments(void) {
  Config out = empty_config();
  TEST_ASSERT_FALSE(config_from_json(NULL, 0, &out));
  TEST_ASSERT_FALSE(config_from_json(VALID_JSON, strlen(VALID_JSON), NULL));
}

static void test_from_json_respects_length(void) {
  // Cutting the valid document short must fail, even though the bytes
  // after the cut are still in memory
  Config out = empty_config();
  TEST_ASSERT_FALSE(
      config_from_json(VALID_JSON, strlen(VALID_JSON) - 4, &out));
}

static void test_from_json_failure_leaves_output_untouched(void) {
  Config out = empty_config();
  out.macro_count = 1;
  out.macros[0].length = 1;
  out.macros[0].strokes[0].key = HID_KEY_Z;
  Config before = out;

  // First macro is valid, second is not
  const char *json =
      "{\"macros\":[{\"strokes\":[" STROKE("HID_A", "") "]},{\"bad\":1}]}";
  TEST_ASSERT_FALSE(config_from_json(json, strlen(json), &out));
  TEST_ASSERT_EQUAL_MEMORY(&before, &out, sizeof(Config));
}

// --- config_to_json ---

static void test_to_json_exact_output(void) {
  config.macro_count = 2;
  config.macros[0] = (Macro){.length = 1,
                             .mode = MACRO_MODE_HOLD,
                             .strokes = {{HID_KEY_SPACE, 0}}};
  config.macros[1] = (Macro){
      .length = 2,
      .mode = MACRO_MODE_SEQUENCE,
      .strokes = {{HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL},
                  {HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL |
                                  KEYBOARD_MODIFIER_RIGHTSHIFT}}};

  char buf[512];
  size_t len = config_to_json(buf, sizeof(buf));

  TEST_ASSERT_EQUAL_STRING(
      "{\"macros\":["
      "{\"mode\":\"hold\",\"strokes\":["
      "{\"key\":\"HID_SPACE\",\"modifiers\":[]}]},"
      "{\"mode\":\"macro\",\"strokes\":["
      "{\"key\":\"HID_C\",\"modifiers\":[\"HID_MOD_LCTRL\"]},"
      "{\"key\":\"HID_V\",\"modifiers\":[\"HID_MOD_LCTRL\",\"HID_MOD_RSHIFT\"]}"
      "]}]}",
      buf);
  TEST_ASSERT_EQUAL_size_t(strlen(buf), len);
}

static void test_to_json_no_macros(void) {
  char buf[64];
  config_to_json(buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("{\"macros\":[]}", buf);
}

static void test_to_json_unknown_key(void) {
  config.macro_count = 1;
  config.macros[0] = (Macro){.length = 1, .strokes = {{0xE7, 0}}};

  char buf[256];
  config_to_json(buf, sizeof(buf));
  TEST_ASSERT_NOT_NULL(strstr(buf, "\"key\":\"HID_UNKNOWN\""));
}

static void test_round_trip_to_json_then_from_json(void) {
  Config original = empty_config();
  TEST_ASSERT_TRUE(
      config_from_json(VALID_JSON, strlen(VALID_JSON), &original));
  config = original;

  char buf[1024];
  size_t len = config_to_json(buf, sizeof(buf));

  Config parsed = empty_config();
  TEST_ASSERT_TRUE(config_from_json(buf, len, &parsed));
  TEST_ASSERT_EQUAL_MEMORY(&original, &parsed, sizeof(Config));
}

// The largest JSON config_to_json can produce: every key has 5 strokes using
// the longest key name and all 8 modifiers.
static Config largest_config(void) {
  Config c = empty_config();
  c.macro_count = 3;
  for (int m = 0; m < 3; m++) {
    c.macros[m].length = MACRO_MAX_STROKES;
    for (int s = 0; s < MACRO_MAX_STROKES; s++) {
      c.macros[m].strokes[s] = (KeyStroke){HID_KEY_BACKSPACE, 0xFF};
    }
  }
  return c;
}

// Removes every `"mode":"macro",` (the parser defaults to that mode), giving
// the shortest JSON body that produces the same config.
static size_t strip_default_mode(char *json) {
  const char *needle = "\"mode\":\"macro\",";
  size_t needle_len = strlen(needle);
  char *hit;
  while ((hit = strstr(json, needle)) != NULL) {
    memmove(hit, hit + needle_len, strlen(hit + needle_len) + 1);
  }
  return strlen(json);
}

static void test_largest_config_round_trips(void) {
  config = largest_config();

  static char buf[CONFIG_JSON_MAX_SIZE];
  size_t len = config_to_json(buf, sizeof(buf));
  TEST_ASSERT_LESS_THAN_size_t(sizeof(buf), len);
  TEST_ASSERT_EQUAL_size_t(strlen(buf), len);

  Config parsed = empty_config();
  TEST_ASSERT_TRUE(config_from_json(buf, len, &parsed));
  TEST_ASSERT_EQUAL_MEMORY(&config, &parsed, sizeof(Config));
}

// Any config the device accepts over POST must also fit the GET response
// buffer, or GET /api/config overflows it.
static void test_any_postable_config_fits_get_response(void) {
  // Start from the largest config and drop modifiers until the shortest
  // JSON body for it fits a POST.
  config = largest_config();
  static char body[8192];
  size_t body_len;
  int m = 0, s = 0;
  while (true) {
    config_to_json(body, sizeof(body));
    body_len = strip_default_mode(body);
    if (body_len < POST_BUFFER_SIZE)
      break;
    // Clear the highest modifier bit of the next stroke, round robin
    uint8_t *mod = &config.macros[m].strokes[s].modifier;
    for (int bit = 7; bit >= 0; bit--) {
      if (*mod & (1u << bit)) {
        *mod &= (uint8_t) ~(1u << bit);
        break;
      }
    }
    if (++s == MACRO_MAX_STROKES) {
      s = 0;
      m = (m + 1) % 3;
    }
  }

  // The device accepts this body
  Config posted = empty_config();
  TEST_ASSERT_TRUE(config_from_json(body, body_len, &posted));
  config = posted;

  // Serialize into a buffer the size of fs.c's, with a guard after it
  struct {
    char response[CONFIG_JSON_MAX_SIZE];
    char guard[1024];
  } out;
  memset(&out, 0, sizeof(out));
  memset(out.guard, 0xA5, sizeof(out.guard));

  size_t len = config_to_json(out.response, sizeof(out.response));

  for (size_t i = 0; i < sizeof(out.guard); i++) {
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xA5, (uint8_t)out.guard[i],
                                   "config_to_json wrote past its buffer");
  }
  TEST_ASSERT_LESS_THAN_size_t(sizeof(out.response), len);
}

// A too-small buffer gets a cut-short, NUL-terminated string and nothing
// past its end is touched.
static void test_to_json_never_writes_past_small_buffer(void) {
  config = largest_config();

  static char full[CONFIG_JSON_MAX_SIZE];
  config_to_json(full, sizeof(full));

  static const size_t sizes[] = {1, 2, 5, 11, 12, 50, 100, 777, 2048};
  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    size_t size = sizes[i];
    static char buf[4096];
    memset(buf, 0xA5, sizeof(buf));

    size_t len = config_to_json(buf, size);

    TEST_ASSERT_EQUAL_size_t(size - 1, len);
    TEST_ASSERT_EQUAL_size_t(len, strlen(buf));
    if (len > 0) {
      TEST_ASSERT_EQUAL_MEMORY(full, buf, len); // a prefix of the full output
    }
    for (size_t j = size; j < size + 64; j++) {
      TEST_ASSERT_EQUAL_HEX8(0xA5, (uint8_t)buf[j]);
    }
  }
}

static void test_to_json_zero_size_buffer_writes_nothing(void) {
  config = largest_config();
  char buf[4] = {'x', 'x', 'x', 'x'};
  TEST_ASSERT_EQUAL_size_t(0, config_to_json(buf, 0));
  TEST_ASSERT_EQUAL_CHAR('x', buf[0]);
}

// --- httpd_post_* ---

typedef struct {
  struct pbuf pbufs[8];
} Chain;

// Splits text into a chain of pbufs of at most chunk bytes each
static struct pbuf *make_chain(Chain *chain, const char *text, size_t chunk) {
  size_t total = strlen(text);
  size_t n = 0;
  for (size_t off = 0; off < total; off += chunk, n++) {
    TEST_ASSERT_LESS_THAN_size_t(8, n);
    size_t len = total - off < chunk ? total - off : chunk;
    chain->pbufs[n] = (struct pbuf){.next = NULL,
                                    .payload = (void *)(text + off),
                                    .tot_len = (u16_t)(total - off),
                                    .len = (u16_t)len};
    if (n > 0)
      chain->pbufs[n - 1].next = &chain->pbufs[n];
  }
  return &chain->pbufs[0];
}

static char response_uri[32];

static void post_begin(const char *uri, int content_len) {
  u8_t auto_wnd = 1;
  memset(response_uri, 0, sizeof(response_uri));
  httpd_post_begin(NULL, uri, "", 0, content_len, response_uri,
                   sizeof(response_uri), &auto_wnd);
}

static void post_body(const char *body, size_t chunk) {
  Chain chain;
  struct pbuf *p = make_chain(&chain, body, chunk);
  TEST_ASSERT_EQUAL_INT(ERR_OK, httpd_post_receive_data(NULL, p));
  TEST_ASSERT_EQUAL_PTR(p, fake_pbuf_last_freed);
}

static void post_finish(void) {
  httpd_post_finished(NULL, response_uri, sizeof(response_uri));
}

static void assert_saved_config_matches_global(void) {
  TEST_ASSERT_EQUAL_INT(1, fake_flash_program_count);
  ConfigStorage stored;
  load_config(&stored);
  TEST_ASSERT_TRUE(config_is_valid(&stored));
  TEST_ASSERT_EQUAL_HEX32(CONFIG_MAGIC, stored.header.magic);
  TEST_ASSERT_EQUAL_MEMORY(&config, &stored.config, sizeof(Config));
}

static void test_post_valid_config_updates_and_saves(void) {
  post_begin("/api/config", (int)strlen(VALID_JSON));
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_UINT16(3, config.macro_count);
  TEST_ASSERT_EQUAL_HEX8(HID_KEY_C, config.macros[0].strokes[0].key);
  TEST_ASSERT_EQUAL_STRING("/api/config", response_uri);
  TEST_ASSERT_EQUAL_INT(1, fake_pbuf_free_count);
  assert_saved_config_matches_global();
}

static void test_post_body_split_across_pbuf_chain(void) {
  post_begin("/api/config", (int)strlen(VALID_JSON));
  post_body(VALID_JSON, 60); // several pbufs in one chain
  post_finish();

  TEST_ASSERT_EQUAL_UINT16(3, config.macro_count);
  assert_saved_config_matches_global();
}

static void test_post_body_split_across_receive_calls(void) {
  size_t total = strlen(VALID_JSON);
  size_t half = total / 2;
  static char first[1024], second[1024];
  memcpy(first, VALID_JSON, half);
  first[half] = '\0';
  strcpy(second, VALID_JSON + half);

  post_begin("/api/config", (int)total);
  post_body(first, 4096);
  post_body(second, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_INT(2, fake_pbuf_free_count);
  TEST_ASSERT_EQUAL_UINT16(3, config.macro_count);
  assert_saved_config_matches_global();
}

static void test_post_invalid_json_keeps_config(void) {
  config.macro_count = 1;
  config.macros[0].length = 1;
  config.macros[0].strokes[0].key = HID_KEY_Z;
  Config before = config;

  const char *body = "{\"macros\":[{\"strokes\":[" STROKE("HID_NOPE", "") "]}]}";
  post_begin("/api/config", (int)strlen(body));
  post_body(body, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_MEMORY(&before, &config, sizeof(Config));
  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
}

static void test_post_to_other_uri_is_ignored(void) {
  post_begin("/api/other", (int)strlen(VALID_JSON));
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_UINT16(0, config.macro_count);
  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
  TEST_ASSERT_EQUAL_INT(1, fake_pbuf_free_count); // still freed
}

static void test_post_with_zero_content_length_is_ignored(void) {
  post_begin("/api/config", 0);
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
}

static void test_post_with_negative_content_length_is_ignored(void) {
  post_begin("/api/config", -1);
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
}

static void test_post_with_too_large_content_length_is_ignored(void) {
  post_begin("/api/config", POST_BUFFER_SIZE);
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
}

static void test_post_body_larger_than_buffer_is_rejected(void) {
  // Valid JSON padded with trailing whitespace past the buffer size,
  // sent in 400-byte pbufs over several receive calls
  static char body[POST_BUFFER_SIZE + 200];
  size_t json_len = strlen(VALID_JSON);
  memcpy(body, VALID_JSON, json_len);
  memset(body + json_len, ' ', sizeof(body) - 1 - json_len);
  body[sizeof(body) - 1] = '\0';

  post_begin("/api/config", 100); // client lied about the length
  size_t sent = 0;
  while (sent < sizeof(body) - 1) {
    static char part[1001];
    size_t n = sizeof(body) - 1 - sent < 1000 ? sizeof(body) - 1 - sent : 1000;
    memcpy(part, body + sent, n);
    part[n] = '\0';
    post_body(part, 400);
    sent += n;
  }
  post_finish();

  TEST_ASSERT_EQUAL_INT(0, fake_flash_program_count);
  TEST_ASSERT_EQUAL_UINT16(0, config.macro_count);
}

static void test_post_state_resets_between_requests(void) {
  // A rejected request must not poison the next one
  post_begin("/api/other", 10);
  post_body("garbage", 4096);
  post_finish();

  post_begin("/api/config", (int)strlen(VALID_JSON));
  post_body(VALID_JSON, 4096);
  post_finish();

  TEST_ASSERT_EQUAL_UINT16(3, config.macro_count);
  TEST_ASSERT_EQUAL_INT(1, fake_flash_program_count);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_from_json_valid_document);
  RUN_TEST(test_from_json_empty_macros);
  RUN_TEST(test_from_json_rejects_more_macros_than_keys);
  RUN_TEST(test_from_json_rejects_trailing_garbage);
  RUN_TEST(test_from_json_allows_trailing_whitespace);
  RUN_TEST(test_from_json_rejects_wrong_top_level_property);
  RUN_TEST(test_from_json_rejects_unterminated);
  RUN_TEST(test_from_json_rejects_null_arguments);
  RUN_TEST(test_from_json_respects_length);
  RUN_TEST(test_from_json_failure_leaves_output_untouched);

  RUN_TEST(test_to_json_exact_output);
  RUN_TEST(test_to_json_no_macros);
  RUN_TEST(test_to_json_unknown_key);
  RUN_TEST(test_round_trip_to_json_then_from_json);
  RUN_TEST(test_largest_config_round_trips);
  RUN_TEST(test_any_postable_config_fits_get_response);
  RUN_TEST(test_to_json_never_writes_past_small_buffer);
  RUN_TEST(test_to_json_zero_size_buffer_writes_nothing);

  RUN_TEST(test_post_valid_config_updates_and_saves);
  RUN_TEST(test_post_body_split_across_pbuf_chain);
  RUN_TEST(test_post_body_split_across_receive_calls);
  RUN_TEST(test_post_invalid_json_keeps_config);
  RUN_TEST(test_post_to_other_uri_is_ignored);
  RUN_TEST(test_post_with_zero_content_length_is_ignored);
  RUN_TEST(test_post_with_negative_content_length_is_ignored);
  RUN_TEST(test_post_with_too_large_content_length_is_ignored);
  RUN_TEST(test_post_body_larger_than_buffer_is_rejected);
  RUN_TEST(test_post_state_resets_between_requests);
  return UNITY_END();
}
