#include "macro.h"

#include "logger.h"

#include "hid_report.h"
#include "pico/time.h"

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

// sets macro to current macro to begin macro process
bool macro_start(const Macro *macro) {
  if (macro == NULL || macro->length == 0)
    return false;
  if (macro_running())
    return false;

  current_macro = macro;
  current_stroke = 0;
  state = MACRO_PRESS;
  next_action = get_absolute_time();
  return true;
}

// Executes macro if current macro is set
void macro_task(void) {
  if (state == MACRO_IDLE) {
    return;
  }

  // Wait until the previous press/release has reached the host, so strokes
  // are never merged away
  if (hid_report_pending()) {
    return;
  }
  if (absolute_time_diff_us(get_absolute_time(), next_action) > 0) {
    return;
  }

  KeyStroke stroke = current_macro->strokes[current_stroke];

  switch (state) {

  case MACRO_PRESS: {
    log_info("Pressed Key %d", stroke.key);

    hid_report_set_macro(stroke.key, stroke.modifier);

    state = MACRO_RELEASE;
    next_action = delayed_by_ms(get_absolute_time(), MACRO_KEY_DELAY_MS);

    break;
  }

  case MACRO_RELEASE:

    hid_report_clear_macro();

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
