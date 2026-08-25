#ifndef KEY_CONFIG_H
#define KEY_CONFIG_H

#include "macro.h"
#include "tusb.h"
#include <stdint.h>

/*
 * Key 0: types "Hello"
 */
static const Macro hello_macro = {.length = 5,
                                  .strokes = {
                                      {HID_KEY_H, 0},
                                      {HID_KEY_E, 0},
                                      {HID_KEY_L, 0},
                                      {HID_KEY_L, 0},
                                      {HID_KEY_O, 0},
                                  }};

/*
 * Key 1: Ctrl + Shift + C
 */
static const Macro ctrl_shift_c_macro = {
    .length = 1,
    .strokes = {
        // {HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL |
        // KEYBOARD_MODIFIER_LEFTSHIFT},
        {HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL},
    }};

/*
 * Physical key -> macro mapping
 */
static const Macro *key_macros[] = {
    &ctrl_shift_c_macro,
    &hello_macro,
};

#endif
