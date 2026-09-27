#include "config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * JSON parser class/state
 */
typedef struct {
  const char *data;
  size_t length;
  size_t position;
} JsonParser;

/*
 * Method to skip JSON whitespace.
 */
void skip_whitespace(JsonParser *parser);

/*
 * Consume a specific character.
 */
bool consume_char(JsonParser *parser, char expected);

/*
 * Parse a JSON string into a buffer.
 *
 * This handles the JSON escapes needed for strings, although your
 * HID names don't currently contain any characters requiring escaping.
 */
bool parse_string(JsonParser *parser, char *buffer, size_t buffer_size);

/*
 * Parse modifiers. Example:
 *
 * "modifiers": [
 *     "HID_MOD_LCTRL",
 *     "HID_MOD_LSHIFT"
 * ]
 */
bool parse_modifiers(JsonParser *parser, uint8_t *modifiers);

/*
 * Parse one stroke. Example:
 *
 * {
 *     "key": "HID_C",
 *     "modifiers": [...]
 * }
 */
bool parse_stroke(JsonParser *parser, KeyStroke *stroke);

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
bool parse_strokes(JsonParser *parser, Macro *macro);

/*
 * Parse one macro:
 *
 * {
 *     "strokes": [...]
 * }
 */
bool parse_macro(JsonParser *parser, Macro *macro);
