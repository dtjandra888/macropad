#ifndef FAKE_MACRO_H
#define FAKE_MACRO_H

#include "macro.h"

void fake_macro_reset(void);

extern bool fake_macro_start_result; // what macro_start() returns
extern int fake_macro_start_count;    // calls, including rejected ones
extern const Macro *fake_macro_last_started;

#endif
