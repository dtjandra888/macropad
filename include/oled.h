#ifndef OLED_H
#define OLED_H

#include <stdbool.h>
#include <stdint.h>

void oled_init(bool reset);

void oled_clear(void);
void oled_draw_pixel(int x, int y, int color);
void oled_draw_char(int x, int y, char c);
void oled_draw_string(int x, int y, const char *str);
void oled_update(void);

#endif
