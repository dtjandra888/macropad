
#include "config_api.h"
#include "config.h"
#include "hid_json.h"
#include "logger.h"

#include <stdio.h>

extern Config config;

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
