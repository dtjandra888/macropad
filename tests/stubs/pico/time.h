// Test stand-in for pico/time.h: time is a plain microsecond counter driven by
// fakes/fake_time.c.
#ifndef PICO_TIME_H_STUB
#define PICO_TIME_H_STUB

#include <stdint.h>

typedef uint64_t absolute_time_t;

absolute_time_t get_absolute_time(void);

static inline int64_t absolute_time_diff_us(absolute_time_t from,
                                            absolute_time_t to) {
  return (int64_t)(to - from);
}

static inline absolute_time_t delayed_by_ms(absolute_time_t t, uint32_t ms) {
  return t + (uint64_t)ms * 1000u;
}

void busy_wait_us(uint64_t us);

#endif
