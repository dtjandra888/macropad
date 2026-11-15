#ifndef FAKE_KEY_H
#define FAKE_KEY_H

#include "key.h"
#include "macropad_config.h"

// key_is_pressed(i) returns fake_key_pressed[i]
extern bool fake_key_pressed[KEY_ROWS * KEY_COLS];

#endif
