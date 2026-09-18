#ifndef KEY_CONFIG_H
#define KEY_CONFIG_H

#include "macro.h"
#include "config.h"

#include "tusb.h"

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
static const Macro ctrl_c_macro = {
    .length = 1,
    .strokes = {
        {HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL},
    }};

static const Macro ctrl_v_macro = {
    .length = 1,
    .strokes = {
        {HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL},
    }};

/*
 * Physical key -> macro mapping
 */
static const Config default_config = {
    .macro_count = 3,
    .macros = {
        [0] = ctrl_c_macro,
        [1] = ctrl_v_macro,
        [2] = hello_macro,
    },
};

#endif
