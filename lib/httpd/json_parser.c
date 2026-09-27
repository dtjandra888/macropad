#include "json_parser.h"

#include "hid_json.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Skip JSON whitespace.
 */
void skip_whitespace(JsonParser *parser) {
  while (parser->position < parser->length) {
    char c = parser->data[parser->position];

    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      break;
    }

    parser->position++;
  }
}

/*
 * Consume a specific character.
 */
bool consume_char(JsonParser *parser, char expected) {
  skip_whitespace(parser);

  if (parser->position >= parser->length) {
    return false;
  }

  if (parser->data[parser->position] != expected) {
    return false;
  }

  parser->position++;
  return true;
}

/*
 * Parse a JSON string into a buffer.
 *
 * This handles the JSON escapes needed for strings, although your
 * HID names don't currently contain any characters requiring escaping.
 */
bool parse_string(JsonParser *parser, char *buffer, size_t buffer_size) {
  skip_whitespace(parser);

  if (parser->position >= parser->length ||
      parser->data[parser->position] != '"') {
    return false;
  }

  parser->position++;

  size_t length = 0;

  while (parser->position < parser->length) {
    char c = parser->data[parser->position++];

    if (c == '"') {
      if (length >= buffer_size) {
        return false;
      }

      buffer[length] = '\0';
      return true;
    }

    if ((unsigned char)c < 0x20) {
      return false;
    }

    if (c == '\\') {
      if (parser->position >= parser->length) {
        return false;
      }

      char escaped = parser->data[parser->position++];

      switch (escaped) {
      case '"':
        c = '"';
        break;

      case '\\':
        c = '\\';
        break;

      case '/':
        c = '/';
        break;

      case 'b':
        c = '\b';
        break;

      case 'f':
        c = '\f';
        break;

      case 'n':
        c = '\n';
        break;

      case 'r':
        c = '\r';
        break;

      case 't':
        c = '\t';
        break;

      /*
       * We don't need to support \uXXXX for your HID names.
       */
      case 'u':
        return false;

      default:
        return false;
      }
    }

    if (length + 1 >= buffer_size) {
      return false;
    }

    buffer[length++] = c;
  }

  return false;
}

/*
 * Parse:
 *
 * "modifiers": [
 *     "HID_MOD_LCTRL",
 *     "HID_MOD_LSHIFT"
 * ]
 */
bool parse_modifiers(JsonParser *parser, uint8_t *modifiers) {
  char modifier_name[32];

  *modifiers = 0;

  if (!consume_char(parser, '[')) {
    return false;
  }

  skip_whitespace(parser);

  /*
   * Empty modifiers array.
   */
  if (parser->position < parser->length &&
      parser->data[parser->position] == ']') {
    parser->position++;
    return true;
  }

  while (true) {
    if (!parse_string(parser, modifier_name, sizeof(modifier_name))) {
      return false;
    }

    uint8_t modifier;

    if (!string_to_modifier(modifier_name, &modifier)) {
      return false;
    }

    *modifiers |= modifier;

    skip_whitespace(parser);

    if (parser->position >= parser->length) {
      return false;
    }

    char c = parser->data[parser->position];

    if (c == ']') {
      parser->position++;
      return true;
    }

    if (c != ',') {
      return false;
    }

    parser->position++;
  }
}

/*
 * Parse one stroke:
 *
 * {
 *     "key": "HID_C",
 *     "modifiers": [...]
 * }
 */
bool parse_stroke(JsonParser *parser, KeyStroke *stroke) {
  char property[16];
  char key_name[32];

  if (!consume_char(parser, '{')) {
    return false;
  }

  /*
   * First property must be "key".
   */
  if (!parse_string(parser, property, sizeof(property))) {
    return false;
  }

  if (strcmp(property, "key") != 0) {
    return false;
  }

  if (!consume_char(parser, ':')) {
    return false;
  }

  if (!parse_string(parser, key_name, sizeof(key_name))) {
    return false;
  }

  if (!string_to_hid_key(key_name, &stroke->key)) {
    return false;
  }

  /*
   * key and modifiers are separated by a comma.
   */
  if (!consume_char(parser, ',')) {
    return false;
  }

  /*
   * Second property must be "modifiers".
   */
  if (!parse_string(parser, property, sizeof(property))) {
    return false;
  }

  if (strcmp(property, "modifiers") != 0) {
    return false;
  }

  if (!consume_char(parser, ':')) {
    return false;
  }

  if (!parse_modifiers(parser, &stroke->modifier)) {
    return false;
  }

  /*
   * End of stroke object.
   */
  if (!consume_char(parser, '}')) {
    return false;
  }

  return true;
}

/*
 * Parse:
 *
 * "strokes": [
 *     {
 *         "key": "...",
 *         "modifiers": [...]
 *     },
 *     ...
 * ]
 */
bool parse_strokes(JsonParser *parser, Macro *macro) {
  macro->length = 0;

  if (!consume_char(parser, '[')) {
    return false;
  }

  skip_whitespace(parser);

  /*
   * Empty strokes array.
   */
  if (parser->position < parser->length &&
      parser->data[parser->position] == ']') {
    parser->position++;
    return true;
  }

  while (true) {
    /*
     * Don't allow the JSON to overflow the Macro.
     */
    if (macro->length >= MACRO_MAX_STROKES) {
      return false;
    }

    if (!parse_stroke(parser, &macro->strokes[macro->length])) {
      return false;
    }

    macro->length++;

    skip_whitespace(parser);

    if (parser->position >= parser->length) {
      return false;
    }

    char c = parser->data[parser->position];

    if (c == ']') {
      parser->position++;
      return true;
    }

    if (c != ',') {
      return false;
    }

    parser->position++;
  }
}

/*
 * Parse one macro:
 *
 * {
 *     "strokes": [...]
 * }
 */
bool parse_macro(JsonParser *parser, Macro *macro) {
  char property[16];

  if (!consume_char(parser, '{')) {
    return false;
  }

  /*
   * The only property in a macro is "strokes".
   */
  if (!parse_string(parser, property, sizeof(property))) {
    return false;
  }

  if (strcmp(property, "strokes") != 0) {
    return false;
  }

  if (!consume_char(parser, ':')) {
    return false;
  }

  if (!parse_strokes(parser, macro)) {
    return false;
  }

  if (!consume_char(parser, '}')) {
    return false;
  }

  return true;
}
