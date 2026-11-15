#ifndef FAKE_OLED_H
#define FAKE_OLED_H

#include "oled.h"

#define FAKE_OLED_MAX_RECTS 16

typedef struct {
  int x, y, w, h;
  bool filled;
} FakeRect;

void fake_oled_reset(void);

// Rects drawn since the last clear
extern FakeRect fake_oled_rects[FAKE_OLED_MAX_RECTS];
extern int fake_oled_rect_count;
extern int fake_oled_update_count;

#endif
