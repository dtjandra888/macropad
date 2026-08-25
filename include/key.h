#ifndef KEY_H
#define KEY_H

void key_init(void);
void key_scan(void);
void send_hid_report(void);
void process_key_events(void);

#endif
