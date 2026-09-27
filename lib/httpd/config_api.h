#ifndef CONFIG_API_H
#define CONFIG_API_H

#include "config.h"

#include <stddef.h>
#include <stdbool.h>

size_t config_to_json(char *buffer, size_t size);
bool config_from_json(const char *json, size_t length, Config *out);

#endif
