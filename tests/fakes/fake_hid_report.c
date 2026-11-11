#include "fake_hid_report.h"

#include <string.h>

bool fake_hid_report_pending;
HidEvent fake_hid_events[FAKE_HID_MAX_EVENTS];
int fake_hid_event_count;

void fake_hid_report_reset(void) {
  fake_hid_report_pending = false;
  memset(fake_hid_events, 0, sizeof(fake_hid_events));
  fake_hid_event_count = 0;
}

static void record(HidEvent event) {
  if (fake_hid_event_count < FAKE_HID_MAX_EVENTS) {
    fake_hid_events[fake_hid_event_count] = event;
  }
  fake_hid_event_count++;
}

void hid_report_hold(uint8_t slot, uint8_t key, uint8_t modifiers) {
  record((HidEvent){HID_EVENT_HOLD, slot, key, modifiers});
}

void hid_report_release(uint8_t slot) {
  record((HidEvent){HID_EVENT_RELEASE, slot, 0, 0});
}

void hid_report_set_macro(uint8_t key, uint8_t modifiers) {
  record((HidEvent){HID_EVENT_SET_MACRO, 0, key, modifiers});
}

void hid_report_clear_macro(void) {
  record((HidEvent){HID_EVENT_CLEAR_MACRO, 0, 0, 0});
}

bool hid_report_pending(void) { return fake_hid_report_pending; }

void hid_report_task(void) {}
