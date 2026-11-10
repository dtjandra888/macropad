#ifndef HID_REPORT_H
#define HID_REPORT_H

// Owns the USB keyboard report. Held keys and the running macro each register
// what they want pressed; the merged result is sent only when it changes.
// This is the only code that should call tud_hid_keyboard_report().

#include <stdbool.h>
#include <stdint.h>

// Held keys, one slot per physical key (index r * KEY_COLS + c)
void hid_report_hold(uint8_t slot, uint8_t key, uint8_t modifiers);
void hid_report_release(uint8_t slot);

// The stroke the macro player is currently pressing
void hid_report_set_macro(uint8_t key, uint8_t modifiers);
void hid_report_clear_macro(void);

// True while the merged state hasn't been sent to the host yet
bool hid_report_pending(void);

// Sends the merged report if it changed and the host is ready
void hid_report_task(void);

#endif
