#ifndef WS2812_H
#define WS2812_H

#include "ws2812.pio.h"

void put_pixel(PIO pio, uint sm, uint32_t pixel_grb);

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

#endif
