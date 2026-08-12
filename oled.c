
#include "oled.h"

#include "font5x7.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include <string.h>

// -------------------------
// Pin Definitions
// -------------------------
#define OLED_SPI_PORT spi1
#define OLED_PIN_SCK 10
#define OLED_PIN_MOSI 11
#define OLED_PIN_DC 12
#define OLED_PIN_CS 13
#define OLED_PIN_RST 14

static uint8_t framebuffer[1024];

static void oled_command(uint8_t cmd) {
  gpio_put(OLED_PIN_CS, 0);
  gpio_put(OLED_PIN_DC, 0);

  spi_write_blocking(OLED_SPI_PORT, &cmd, 1);

  gpio_put(OLED_PIN_CS, 1);
}

static void oled_data(const uint8_t *data, size_t length) {
  gpio_put(OLED_PIN_DC, 1); // Data mode
  gpio_put(OLED_PIN_CS, 0);

  spi_write_blocking(OLED_SPI_PORT, data, length);

  gpio_put(OLED_PIN_CS, 1);
}

static const struct Font *find_char(char c) {
  int i = 0;

  while (font[i].letter != 0) {
    if (font[i].letter == c)
      return &font[i];

    i++;
  }

  return NULL;
}

void oled_update(void) {
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

void oled_init(bool reset) {
  // Initialize SPI
  spi_init(OLED_SPI_PORT, 1000 * 1000); // 1 MHz

  gpio_set_function(OLED_PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(OLED_PIN_MOSI, GPIO_FUNC_SPI);

  // Configure GPIO pins
  gpio_init(OLED_PIN_DC);
  gpio_set_dir(OLED_PIN_DC, GPIO_OUT);

  gpio_init(OLED_PIN_CS);
  gpio_set_dir(OLED_PIN_CS, GPIO_OUT);

  gpio_init(OLED_PIN_RST);
  gpio_set_dir(OLED_PIN_RST, GPIO_OUT);

  gpio_put(OLED_PIN_CS, 1);

  // Hardware Reset
  if (reset) {
    gpio_put(OLED_PIN_RST, 1);
    sleep_ms(10);

    gpio_put(OLED_PIN_RST, 0);
    sleep_ms(10);

    gpio_put(OLED_PIN_RST, 1);
    sleep_ms(100);
  }

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

void oled_clear(void) {
  memset(framebuffer, 0, sizeof(framebuffer));

  oled_update();
}

void oled_draw_pixel(int x, int y, int color) {
  if (x < 0 || x >= 128)
    return;
  if (y < 0 || y >= 64)
    return;

  int page = y / 8;
  int bit = y % 8;

  int index = page * 128 + x;
  if (color) {
    framebuffer[index] |= (1 << bit);
  } else {
    framebuffer[index] &= ~(1 << bit);
  }
}

void oled_draw_char(int x, int y, char c) {
  const struct Font *glyph = find_char(c);

  if (glyph == NULL) {
    return;
  }
  for (int r = 0; r < 7; r++) {
    for (int c = 0; c < 5; c++) {
      if (glyph->code[r][c] != '#') {
        continue;
      }
      oled_draw_pixel(x + c, y + r, 1);
    }
  }
}

void oled_draw_string(int x, int y, const char *str) {
  while (*str) {
    oled_draw_char(x, y, *str);

    // draw spacing between chars
    x += 6;

    str++;
  }
}
