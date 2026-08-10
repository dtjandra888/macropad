#ifndef KEY_H
#define KEY_H

#define KEY_ROWS 1
#define KEY_COLS 1

#define KEY_ROW_PINS {2}
#define KEY_COL_PINS {3}

void key_init(void);
void key_scan(void);
void send_hid_report(void);

#endif
