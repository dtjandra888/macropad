#ifndef CONFIG_API_H
#define CONFIG_API_H

#include "config.h"

#include <stddef.h>
#include <stdbool.h>

// Buffer size that fits the JSON of the largest possible Config (3 macros x 5
// strokes, longest key names, all modifiers: 2559 bytes), with headroom.
#define CONFIG_JSON_MAX_SIZE 3072

// Writes the global config as JSON. Never writes past size; output that
// doesn't fit is cut short. Returns the number of characters written.
size_t config_to_json(char *buffer, size_t size);
bool config_from_json(const char *json, size_t length, Config *out);

#endif
