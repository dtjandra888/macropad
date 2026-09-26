// Utility functions for converting JSON to HID codes

#include "hid_json.h"
#include "tusb.h"

const char *hid_key_to_string(uint8_t key) {
  switch (key) {
  case HID_KEY_A:
    return "HID_A";
  case HID_KEY_B:
    return "HID_B";
  case HID_KEY_C:
    return "HID_C";
  case HID_KEY_D:
    return "HID_D";
  case HID_KEY_E:
    return "HID_E";
  case HID_KEY_F:
    return "HID_F";
  case HID_KEY_G:
    return "HID_G";
  case HID_KEY_H:
    return "HID_H";
  case HID_KEY_I:
    return "HID_I";
  case HID_KEY_J:
    return "HID_J";
  case HID_KEY_K:
    return "HID_K";
  case HID_KEY_L:
    return "HID_L";
  case HID_KEY_M:
    return "HID_M";
  case HID_KEY_N:
    return "HID_N";
  case HID_KEY_O:
    return "HID_O";
  case HID_KEY_P:
    return "HID_P";
  case HID_KEY_Q:
    return "HID_Q";
  case HID_KEY_R:
    return "HID_R";
  case HID_KEY_S:
    return "HID_S";
  case HID_KEY_T:
    return "HID_T";
  case HID_KEY_U:
    return "HID_U";
  case HID_KEY_V:
    return "HID_V";
  case HID_KEY_W:
    return "HID_W";
  case HID_KEY_X:
    return "HID_X";
  case HID_KEY_Y:
    return "HID_Y";
  case HID_KEY_Z:
    return "HID_Z";

  case HID_KEY_1:
    return "HID_1";
  case HID_KEY_2:
    return "HID_2";
  case HID_KEY_3:
    return "HID_3";
  case HID_KEY_4:
    return "HID_4";
  case HID_KEY_5:
    return "HID_5";
  case HID_KEY_6:
    return "HID_6";
  case HID_KEY_7:
    return "HID_7";
  case HID_KEY_8:
    return "HID_8";
  case HID_KEY_9:
    return "HID_9";
  case HID_KEY_0:
    return "HID_0";

  case HID_KEY_ENTER:
    return "HID_ENTER";
  case HID_KEY_ESCAPE:
    return "HID_ESCAPE";
  case HID_KEY_BACKSPACE:
    return "HID_BACKSPACE";
  case HID_KEY_TAB:
    return "HID_TAB";
  case HID_KEY_SPACE:
    return "HID_SPACE";
  // TODO: Add the rest of the codes
  default:
    return NULL;
  }
}

size_t modifiers_to_json(char *buffer, size_t size, uint8_t modifiers) {
  size_t offset = 0;
  bool first = true;

  offset += snprintf(buffer + offset, size - offset, "[");

  if (modifiers & KEYBOARD_MODIFIER_LEFTCTRL) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_LCTRL\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_LEFTSHIFT) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_LSHIFT\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_LEFTALT) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_LALT\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_LEFTGUI) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_LGUI\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_RIGHTCTRL) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_RCTRL\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_RIGHTSHIFT) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_RSHIFT\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_RIGHTALT) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_RALT\"",
                       first ? "" : ",");
    first = false;
  }

  if (modifiers & KEYBOARD_MODIFIER_RIGHTGUI) {
    offset += snprintf(buffer + offset, size - offset, "%s\"HID_MOD_RGUI\"",
                       first ? "" : ",");
    first = false;
  }

  offset += snprintf(buffer + offset, size - offset, "]");

  return offset;
}
