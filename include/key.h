#ifndef KEY_H
#define KEY_H

#include <stdbool.h>

void key_init(void);
void key_scan(void);
void send_hid_report(void);
void process_key_events(void);

// Debounced state of key `index` (r * KEY_COLS + c, same as config.macros)
bool key_is_pressed(int index);

#endif
