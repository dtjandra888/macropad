#ifndef HID_JSON_H
#define HID_JSON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

const char *hid_key_to_string(uint8_t key);
bool string_to_hid_key(const char *name, uint8_t *key);

size_t modifiers_to_json(char *buffer, size_t size, uint8_t modifiers);
bool string_to_modifier(const char *name, uint8_t *modifier);

#endif
