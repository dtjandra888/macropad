#ifndef KEY_CONFIG_H
#define KEY_CONFIG_H

#include "class/hid/hid.h"
#include "config.h"
#include "macro.h"

#include "tusb.h"

#define KEY_DEBOUNCE_MS 5
#define KEY_DEBOUNCE_US (KEY_DEBOUNCE_MS * 1000)

// Hold-to-repeat: wait this long after the first press before repeating
#define MACRO_REPEAT_DELAY_MS 500
#define MACRO_REPEAT_INTERVAL_MS 50

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
        {HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT},
    }};

static const Macro ctrl_v_macro = {.length = 1,
                                   .strokes = {
                                       {HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL},
                                   }};

/*
 * Physical key -> macro mapping
 */
static const Config default_config = {
    .macro_count = 3,
    .macros =
        {
            [0] = ctrl_c_macro,
            [1] = ctrl_v_macro,
            [2] = hello_macro,
        },
};

#endif
