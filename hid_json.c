// Utility functions for converting JSON to HID codes

#include "hid_json.h"
#include "tusb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  uint8_t key;
  const char *name;
} HidKeyName;

static const HidKeyName hid_keys[] = {
    {HID_KEY_A, "HID_A"},
    {HID_KEY_B, "HID_B"},
    {HID_KEY_C, "HID_C"},
    {HID_KEY_D, "HID_D"},
    {HID_KEY_E, "HID_E"},
    {HID_KEY_F, "HID_F"},
    {HID_KEY_G, "HID_G"},
    {HID_KEY_H, "HID_H"},
    {HID_KEY_I, "HID_I"},
    {HID_KEY_J, "HID_J"},
    {HID_KEY_K, "HID_K"},
    {HID_KEY_L, "HID_L"},
    {HID_KEY_M, "HID_M"},
    {HID_KEY_N, "HID_N"},
    {HID_KEY_O, "HID_O"},
    {HID_KEY_P, "HID_P"},
    {HID_KEY_Q, "HID_Q"},
    {HID_KEY_R, "HID_R"},
    {HID_KEY_S, "HID_S"},
    {HID_KEY_T, "HID_T"},
    {HID_KEY_U, "HID_U"},
    {HID_KEY_V, "HID_V"},
    {HID_KEY_W, "HID_W"},
    {HID_KEY_X, "HID_X"},
    {HID_KEY_Y, "HID_Y"},
    {HID_KEY_Z, "HID_Z"},

    {HID_KEY_1, "HID_1"},
    {HID_KEY_2, "HID_2"},
    {HID_KEY_3, "HID_3"},
    {HID_KEY_4, "HID_4"},
    {HID_KEY_5, "HID_5"},
    {HID_KEY_6, "HID_6"},
    {HID_KEY_7, "HID_7"},
    {HID_KEY_8, "HID_8"},
    {HID_KEY_9, "HID_9"},
    {HID_KEY_0, "HID_0"},

    {HID_KEY_ENTER, "HID_ENTER"},
    {HID_KEY_ESCAPE, "HID_ESCAPE"},
    {HID_KEY_BACKSPACE, "HID_BACKSPACE"},
    {HID_KEY_TAB, "HID_TAB"},
    {HID_KEY_SPACE, "HID_SPACE"},
};

typedef struct {
  uint8_t modifier;
  const char *name;
} HidModifierName;

static const HidModifierName hid_modifiers[] = {
    {KEYBOARD_MODIFIER_LEFTCTRL, "HID_MOD_LCTRL"},
    {KEYBOARD_MODIFIER_LEFTSHIFT, "HID_MOD_LSHIFT"},
    {KEYBOARD_MODIFIER_LEFTALT, "HID_MOD_LALT"},
    {KEYBOARD_MODIFIER_LEFTGUI, "HID_MOD_LGUI"},
    {KEYBOARD_MODIFIER_RIGHTCTRL, "HID_MOD_RCTRL"},
    {KEYBOARD_MODIFIER_RIGHTSHIFT, "HID_MOD_RSHIFT"},
    {KEYBOARD_MODIFIER_RIGHTALT, "HID_MOD_RALT"},
    {KEYBOARD_MODIFIER_RIGHTGUI, "HID_MOD_RGUI"},
};

const char *hid_key_to_string(uint8_t key) {
  for (size_t i = 0; i < sizeof(hid_keys) / sizeof(hid_keys[0]); i++) {
    if (hid_keys[i].key == key) {
      return hid_keys[i].name;
    }
  }

  return NULL;
}

bool string_to_hid_key(const char *name, uint8_t *key) {
  for (size_t i = 0; i < sizeof(hid_keys) / sizeof(hid_keys[0]); i++) {
    if (strcmp(hid_keys[i].name, name) == 0) {
      *key = hid_keys[i].key;
      return true;
    }
  }

  return false;
}

size_t modifiers_to_json(char *buffer, size_t size, uint8_t modifiers) {
  size_t offset = 0;
  bool first = true;

  offset += snprintf(buffer + offset, size - offset, "[");

  for (size_t i = 0; i < sizeof(hid_modifiers) / sizeof(hid_modifiers[0]);
       i++) {

    if (modifiers & hid_modifiers[i].modifier) {
      offset += snprintf(buffer + offset, size - offset, "%s\"%s\"",
                         first ? "" : ",", hid_modifiers[i].name);

      first = false;
    }
  }

  offset += snprintf(buffer + offset, size - offset, "]");

  return offset;
}

bool string_to_modifier(const char *name, uint8_t *modifier) {
  for (size_t i = 0; i < sizeof(hid_modifiers) / sizeof(hid_modifiers[0]);
       i++) {

    if (strcmp(hid_modifiers[i].name, name) == 0) {
      *modifier = hid_modifiers[i].modifier;
      return true;
    }
  }

  return false;
}
