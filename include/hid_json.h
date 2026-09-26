#ifndef HID_JSON_H
#define HID_JSON_H

#include <stdint.h>
#include <stddef.h>

const char *hid_key_to_string(uint8_t key);

// TODO - for POST endpoint
// const char *hid_modifier_to_string(uint8_t modifier);

size_t modifiers_to_json(char *buffer, size_t size, uint8_t modifiers);

#endif
