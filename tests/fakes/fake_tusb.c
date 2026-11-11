#include "fake_tusb.h"

#include <string.h>

bool fake_tud_hid_ready;
bool fake_tud_hid_report_result;
int fake_report_count;
uint8_t fake_report_id;
uint8_t fake_report_modifier;
uint8_t fake_report_keycodes[6];

void fake_tusb_reset(void) {
  fake_tud_hid_ready = true;
  fake_tud_hid_report_result = true;
  fake_report_count = 0;
  fake_report_id = 0;
  fake_report_modifier = 0;
  memset(fake_report_keycodes, 0, sizeof(fake_report_keycodes));
}

bool tud_hid_ready(void) { return fake_tud_hid_ready; }

bool tud_hid_keyboard_report(uint8_t report_id, uint8_t modifier,
                             const uint8_t keycode[6]) {
  fake_report_count++;
  fake_report_id = report_id;
  fake_report_modifier = modifier;
  memcpy(fake_report_keycodes, keycode, 6);
  return fake_tud_hid_report_result;
}
