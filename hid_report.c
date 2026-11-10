#include "hid_report.h"

#include "macropad_config.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include <string.h>

#define SLOT_COUNT (KEY_ROWS * KEY_COLS)
#define MAX_KEYCODES 6

typedef struct {
  bool active;
  uint8_t key;
  uint8_t modifiers;
} PressedKey;

typedef struct {
  uint8_t modifiers;
  uint8_t keycodes[MAX_KEYCODES];
} KeyboardReport;

static PressedKey held[SLOT_COUNT];
static PressedKey macro_stroke;
static KeyboardReport last_sent;

void hid_report_hold(uint8_t slot, uint8_t key, uint8_t modifiers) {
  if (slot >= SLOT_COUNT)
    return;
  held[slot] = (PressedKey){.active = true, .key = key, .modifiers = modifiers};
}

void hid_report_release(uint8_t slot) {
  if (slot >= SLOT_COUNT)
    return;
  held[slot].active = false;
}

void hid_report_set_macro(uint8_t key, uint8_t modifiers) {
  macro_stroke = (PressedKey){.active = true, .key = key, .modifiers = modifiers};
}

void hid_report_clear_macro(void) { macro_stroke.active = false; }

// Adds one pressed key to the report: OR in its modifiers and append its
// keycode unless it is empty, already present, or the report is full.
static void add_to_report(KeyboardReport *report, const PressedKey *pressed) {
  if (!pressed->active)
    return;

  report->modifiers |= pressed->modifiers;

  if (pressed->key == 0)
    return;

  for (int i = 0; i < MAX_KEYCODES; i++) {
    if (report->keycodes[i] == pressed->key)
      return;
    if (report->keycodes[i] == 0) {
      report->keycodes[i] = pressed->key;
      return;
    }
  }
}

static KeyboardReport build_report(void) {
  KeyboardReport report = {0};

  for (int i = 0; i < SLOT_COUNT; i++) {
    add_to_report(&report, &held[i]);
  }
  add_to_report(&report, &macro_stroke);

  return report;
}

bool hid_report_pending(void) {
  KeyboardReport report = build_report();
  return memcmp(&report, &last_sent, sizeof(report)) != 0;
}

void hid_report_task(void) {
  KeyboardReport report = build_report();

  if (memcmp(&report, &last_sent, sizeof(report)) == 0)
    return;
  if (!tud_hid_ready())
    return;

  if (tud_hid_keyboard_report(REPORT_ID_KEYBOARD, report.modifiers,
                              report.keycodes)) {
    last_sent = report;
  }
}
