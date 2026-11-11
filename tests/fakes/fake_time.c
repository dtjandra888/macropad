#include "fake_time.h"

static uint64_t now_us;
int fake_busy_wait_count;

void fake_time_set_us(uint64_t us) { now_us = us; }
void fake_time_advance_us(uint64_t us) { now_us += us; }
void fake_time_advance_ms(uint64_t ms) { now_us += ms * 1000u; }

absolute_time_t get_absolute_time(void) { return now_us; }

void busy_wait_us(uint64_t us) {
  (void)us;
  fake_busy_wait_count++;
}
