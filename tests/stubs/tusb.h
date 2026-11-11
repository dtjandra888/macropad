// Test stand-in for TinyUSB: only the constants and calls our code uses.
#ifndef TUSB_H_STUB
#define TUSB_H_STUB

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "class/hid/hid.h"

bool tud_hid_ready(void);
bool tud_hid_keyboard_report(uint8_t report_id, uint8_t modifier,
                             const uint8_t keycode[6]);

#endif
