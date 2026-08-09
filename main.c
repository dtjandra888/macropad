// Pico stuff
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "logger.h"
#include "oled.h"
#include "ws2812.h"
#include "ws2812.pio.h"

#include <stdio.h>

#define LED_PIN 16

int main() {
  stdio_init_all();


  oled_init(false);
  logger_init();

  log_info("Macropad starting");
  log_info("OLED initialized");

  sleep_ms(5000);

  log_error("Hello World");
  while (true) {
    sleep_ms(1000);
    log_info("Tick");
  }
}

//
//   printf("Hello! from RP2040\n");
//   printf("Initializing ws2812\n");
//
//   oled_init(false);
//   oled_clear();
//   oled_update();
//   sleep_ms(5000);
//
//   PIO pio = pio0;
//   uint sm = 0;
//   uint offset = 0;
//
//   bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
//       &ws2812_program, &pio, &sm, &offset, LED_PIN, 1, true);
//   hard_assert(success);
//
//   printf("Program init\n");
//   ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);
//
//   oled_draw_string(50, 50, "Hello World!");
//   oled_update();
//
//   while (true) {
//     printf("Tick\n");
//     sleep_ms(5000);
//
//     put_pixel(pio, sm, urgb_u32(0xff, 0, 0));
//     sleep_ms(1000);
//     put_pixel(pio, sm, urgb_u32(0, 0xff, 0));
//     sleep_ms(1000);
//     put_pixel(pio, sm, urgb_u32(0, 0, 0xff));
//   }
//
//   pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
//}
