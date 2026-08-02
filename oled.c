
#include "oled.h"

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include <string.h>

// -------------------------
// Pin Definitions
// -------------------------

#define SPI_PORT spi1

#define PIN_SCK 10
#define PIN_MOSI 11
#define PIN_DC 12
#define PIN_CS 13
#define PIN_RST 14

static void oled_command(uint8_t cmd) {
  gpio_put(PIN_CS, 0);
  gpio_put(PIN_DC, 0);

  spi_write_blocking(SPI_PORT, &cmd, 1);

  gpio_put(PIN_CS, 1);
}

static void oled_data(const uint8_t *data, size_t length) {
  gpio_put(PIN_DC, 1); // Data mode
  gpio_put(PIN_CS, 0);

  spi_write_blocking(SPI_PORT, data, length);

  gpio_put(PIN_CS, 1);
}

void oled_init(void) {
  // Initialize SPI
  spi_init(SPI_PORT, 1000 * 1000); // 1 MHz

  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Configure GPIO pins
  gpio_init(PIN_DC);
  gpio_set_dir(PIN_DC, GPIO_OUT);

  gpio_init(PIN_CS);
  gpio_set_dir(PIN_CS, GPIO_OUT);

  gpio_init(PIN_RST);
  gpio_set_dir(PIN_RST, GPIO_OUT);

  gpio_put(PIN_CS, 1);

  // Hardware Reset
  gpio_put(PIN_RST, 1);
  sleep_ms(10);

  gpio_put(PIN_RST, 0);
  sleep_ms(10);

  gpio_put(PIN_RST, 1);
  sleep_ms(100);

  // SSD1315 initialization sequence
  oled_command(0xAE); // Display OFF

  oled_command(0x20);
  oled_command(0x00); // Horizontal addressing mode

  oled_command(0xA1);
  oled_command(0xC8);

  oled_command(0x81);
  oled_command(0x7F);

  oled_command(0xA6);

  oled_command(0xA8);
  oled_command(0x3F);

  oled_command(0xD3);
  oled_command(0x00);

  oled_command(0xD5);
  oled_command(0x80);

  oled_command(0xD9);
  oled_command(0xF1);

  oled_command(0xDA);
  oled_command(0x12);

  oled_command(0xDB);
  oled_command(0x40);

  oled_command(0x8D);
  oled_command(0x14);

  oled_command(0xAF); // Display ON
}

void oled_fill(uint8_t value) {
  uint8_t framebuffer[1024];

  memset(framebuffer, value, sizeof(framebuffer));

  // Column address
  oled_command(0x21);
  oled_command(0);
  oled_command(127);

  // Page address
  oled_command(0x22);
  oled_command(0);
  oled_command(7);

  oled_data(framebuffer, sizeof(framebuffer));
}
