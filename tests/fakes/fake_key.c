#include "fake_key.h"

bool fake_key_pressed[KEY_ROWS * KEY_COLS];

bool key_is_pressed(int index) { return fake_key_pressed[index]; }
