#ifndef HID_JSON_H
#define HID_JSON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

const char *hid_key_to_string(uint8_t key);
bool string_to_hid_key(const char *name, uint8_t *key);

// printf-style append at buffer + *offset that never writes past size. Output
// that doesn't fit is cut short; *offset always stays below size, so the
// buffer is always NUL-terminated.
void json_append(char *buffer, size_t size, size_t *offset,
                 const char *format, ...)
    __attribute__((format(printf, 4, 5)));

// Never writes past size; returns the number of characters written.
size_t modifiers_to_json(char *buffer, size_t size, uint8_t modifiers);
bool string_to_modifier(const char *name, uint8_t *modifier);

#endif
