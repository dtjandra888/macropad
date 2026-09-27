#include "config_api.h"
#include "config.h"
#include "hid_json.h"
#include "json_parser.h"
#include "logger.h"

#include "lwip/err.h"
#include "lwip/pbuf.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define POST_BUFFER_SIZE 2048

extern Config config;

static char post_buffer[POST_BUFFER_SIZE];
static size_t post_length;
static bool post_valid;

size_t config_to_json(char *buffer, size_t size) {
  size_t offset = 0;

  offset += snprintf(buffer + offset, size - offset, "{\"macros\":[");

  for (uint16_t i = 0; i < config.macro_count; i++) {
    const Macro *macro = &config.macros[i];

    if (i > 0) {
      offset += snprintf(buffer + offset, size - offset, ",");
    }

    offset += snprintf(buffer + offset, size - offset, "{\"strokes\":[");

    for (int j = 0; j < macro->length; j++) {
      const KeyStroke *stroke = &macro->strokes[j];

      if (j > 0) {
        offset += snprintf(buffer + offset, size - offset, ",");
      }

      const char *key = hid_key_to_string(stroke->key);

      offset += snprintf(buffer + offset, size - offset,
                         "{\"key\":\"%s\",\"modifiers\":[",
                         key ? key : "HID_UNKNOWN");

      offset +=
          modifiers_to_json(buffer + offset, size - offset, stroke->modifier);

      offset += snprintf(buffer + offset, size - offset, "]}");
    }

    offset += snprintf(buffer + offset, size - offset, "]}");
  }

  offset += snprintf(buffer + offset, size - offset, "]}");

  return offset;
}

bool config_from_json(const char *json, size_t json_length, Config *config) {
  if (json == NULL || config == NULL) {
    return false;
  }

  JsonParser parser = {
      .data = json,
      .length = json_length,
      .position = 0,
  };

  /*
   * Parse into a temporary Config so that the existing
   * configuration isn't modified if parsing fails.
   */
  Config temp = {0};

  char property[16];

  /*
   * {
   */
  if (!consume_char(&parser, '{')) {
    return false;
  }

  /*
   * "macros"
   */
  if (!parse_string(&parser, property, sizeof(property))) {
    return false;
  }

  if (strcmp(property, "macros") != 0) {
    return false;
  }

  /*
   * :
   */
  if (!consume_char(&parser, ':')) {
    return false;
  }

  /*
   * [
   */
  if (!consume_char(&parser, '[')) {
    return false;
  }

  skip_whitespace(&parser);

  /*
   * Empty macros array.
   */
  if (parser.position < parser.length && parser.data[parser.position] == ']') {

    parser.position++;

  } else {
    while (true) {
      /*
       * Don't allow more macros than the Config can hold.
       */
      if (temp.macro_count >= 3) {
        return false;
      }

      if (!parse_macro(&parser, &temp.macros[temp.macro_count])) {
        return false;
      }

      temp.macro_count++;

      skip_whitespace(&parser);

      if (parser.position >= parser.length) {
        return false;
      }

      char c = parser.data[parser.position];

      if (c == ']') {
        parser.position++;
        break;
      }

      if (c != ',') {
        return false;
      }

      parser.position++;
    }
  }

  /*
   * }
   */
  if (!consume_char(&parser, '}')) {
    return false;
  }

  /*
   * There must not be anything after the JSON object other
   * than whitespace.
   */
  skip_whitespace(&parser);

  if (parser.position != parser.length) {
    return false;
  }

  /*
   * Everything succeeded, so commit the new configuration.
   */
  *config = temp;

  return true;
}

// POST methods implemented as required by LWIP
void httpd_post_begin(void *connection, const char *uri,
                      const char *http_request, u16_t http_request_len,
                      int content_len, char *response_uri,
                      u8_t response_uri_len, u8_t *post_auto_wnd) {
  (void)connection;
  (void)http_request;
  (void)http_request_len;
  (void)response_uri;
  (void)response_uri_len;
  (void)post_auto_wnd;

  post_length = 0;
  post_valid = true;

  if (strcmp(uri, "/api/config") != 0) {
    post_valid = false;
    return;
  }

  if (content_len <= 0 || content_len >= POST_BUFFER_SIZE) {
    post_valid = false;
    return;
  }
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
  if (!post_valid) {
    pbuf_free(p);
    return ERR_OK;
  }

  struct pbuf *q = p;

  while (q != NULL) {
    if (post_length + q->len >= POST_BUFFER_SIZE) {
      post_valid = false;
      pbuf_free(p);
      return ERR_OK;
    }

    memcpy(post_buffer + post_length, q->payload, q->len);

    post_length += q->len;

    q = q->next;
  }

  post_buffer[post_length] = '\0';

  pbuf_free(p);

  return ERR_OK;
}

static void save_current_config(void) {
  ConfigStorage storage = {
      .header =
          {
              .magic = CONFIG_MAGIC,
              .size = sizeof(Config),
          },
      .config = config,
  };

  storage.header.crc32 =
      crc32((const uint8_t *)&storage.config, sizeof(Config));

  save_config(&storage);
}

void httpd_post_finished(void *connection, char *response_uri,
                         u8_t response_uri_len) {
  Config new_config;

  if (!post_valid) {
    log_info("POST invalid");
    return;
  }

  log_info("Parsing config");

  if (!config_from_json(post_buffer, post_length, &new_config)) {

    log_info("Invalid config");
    return;
  }

  log_info("Config valid");

  /*
   * Only update the global config after the entire
   * JSON document has been successfully parsed.
   */
  config = new_config;

  save_current_config();

  log_info("Config saved");

  snprintf(response_uri, response_uri_len, "/api/config");
}
