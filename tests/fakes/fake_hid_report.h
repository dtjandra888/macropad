#ifndef FAKE_HID_REPORT_H
#define FAKE_HID_REPORT_H

#include "hid_report.h"

typedef enum {
  HID_EVENT_HOLD,
  HID_EVENT_RELEASE,
  HID_EVENT_SET_MACRO,
  HID_EVENT_CLEAR_MACRO,
} HidEventType;

typedef struct {
  HidEventType type;
  uint8_t slot;
  uint8_t key;
  uint8_t modifiers;
} HidEvent;

#define FAKE_HID_MAX_EVENTS 64

void fake_hid_report_reset(void);

extern bool fake_hid_report_pending; // what hid_report_pending() returns
extern HidEvent fake_hid_events[FAKE_HID_MAX_EVENTS];
extern int fake_hid_event_count;

#endif
