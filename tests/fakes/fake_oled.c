// Records drawing calls instead of driving the SSD1315
#include "fake_oled.h"

#include <string.h>

FakeRect fake_oled_rects[FAKE_OLED_MAX_RECTS];
int fake_oled_rect_count;
int fake_oled_update_count;

void fake_oled_reset(void) {
  memset(fake_oled_rects, 0, sizeof(fake_oled_rects));
  fake_oled_rect_count = 0;
  fake_oled_update_count = 0;
}

void oled_init(void) {}
void oled_clear_buffer(void) { fake_oled_rect_count = 0; }
void oled_clear(void) {
  oled_clear_buffer();
  oled_update();
}
void oled_update(void) { fake_oled_update_count++; }
void oled_draw_pixel(int x, int y, int color) { (void)x, (void)y, (void)color; }
void oled_draw_char(int x, int y, char c) { (void)x, (void)y, (void)c; }
void oled_draw_string(int x, int y, const char *str) {
  (void)x, (void)y, (void)str;
}

void oled_draw_rect(int x, int y, int w, int h, bool filled) {
  if (fake_oled_rect_count < FAKE_OLED_MAX_RECTS)
    fake_oled_rects[fake_oled_rect_count++] = (FakeRect){x, y, w, h, filled};
}
