
#include "ws2812.h"
#include "macropad_config.h"

static PIO pio = pio0;
static uint sm = 0;
static uint offset = 0;

static void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void ws2812_init(void) {
  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
  hard_assert(success);
  ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}

void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
  put_pixel(urgb_u32(r, g, b));
}
