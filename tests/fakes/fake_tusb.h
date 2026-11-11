#ifndef FAKE_TUSB_H
#define FAKE_TUSB_H

#include "tusb.h"

void fake_tusb_reset(void);

extern bool fake_tud_hid_ready;          // what tud_hid_ready() returns
extern bool fake_tud_hid_report_result;  // what tud_hid_keyboard_report() returns

// Recorded tud_hid_keyboard_report() calls
extern int fake_report_count;
extern uint8_t fake_report_id;
extern uint8_t fake_report_modifier;
extern uint8_t fake_report_keycodes[6];

#endif
