#ifndef FAKE_TIME_H
#define FAKE_TIME_H

#include "pico/time.h"

void fake_time_set_us(uint64_t us);
void fake_time_advance_us(uint64_t us);
void fake_time_advance_ms(uint64_t ms);

extern int fake_busy_wait_count;

#endif
