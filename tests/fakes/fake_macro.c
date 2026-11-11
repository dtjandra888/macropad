#include "fake_macro.h"

#include <stddef.h>

bool fake_macro_start_result;
int fake_macro_start_count;
const Macro *fake_macro_last_started;

void fake_macro_reset(void) {
  fake_macro_start_result = true;
  fake_macro_start_count = 0;
  fake_macro_last_started = NULL;
}

bool macro_start(const Macro *macro) {
  fake_macro_start_count++;
  fake_macro_last_started = macro;
  return fake_macro_start_result;
}

void macro_task(void) {}
bool macro_running(void) { return false; }
