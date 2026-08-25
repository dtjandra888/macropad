#include "macro.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include <stddef.h>

#define MACRO_KEY_DELAY_MS 20

typedef enum {
  MACRO_IDLE,
  MACRO_PRESS,
  MACRO_RELEASE,
} MacroState;

static const Macro *current_macro = NULL;
static int current_stroke = 0;
static MacroState state = MACRO_IDLE;
static absolute_time_t next_action;

bool macro_running(void) { return state != MACRO_IDLE; }

// 
void macro_start(const Macro *macro) {
  if (macro == NULL || macro->length == 0)
    return;
  if (macro_running())
    return;

  current_macro = macro;
  current_stroke = 0;
  state = MACRO_PRESS;
  next_action = get_absolute_time();
}

void macro_task(void) {
  if (state == MACRO_IDLE) {
    return;
  }

  if (!tud_hid_ready()) {
    return;
  }
  if (absolute_time_diff_us(get_absolute_time(), next_action) > 0) {
    return;
  }

  KeyStroke stroke = current_macro->strokes[current_stroke];

  switch (state) {

  case MACRO_PRESS: {
    uint8_t keycode[6] = {0};

    keycode[0] = stroke.key;

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, stroke.modifier, keycode);

    state = MACRO_RELEASE;
    next_action = delayed_by_ms(get_absolute_time(), MACRO_KEY_DELAY_MS);

    break;
  }

  case MACRO_RELEASE:

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);

    current_stroke++;

    if (current_stroke >= current_macro->length) {
      current_macro = NULL;
      state = MACRO_IDLE;
    } else {
      state = MACRO_PRESS;

      next_action = delayed_by_ms(get_absolute_time(), MACRO_KEY_DELAY_MS);
    }

    break;

  case MACRO_IDLE:
    break;
  }
}
