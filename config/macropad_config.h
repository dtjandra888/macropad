#ifndef MACROPAD_CONFIG_H
#define MACROPAD_CONFIG_H

// Key matrix
#define KEY_ROWS 1
#define KEY_COLS 1

#define KEY_ROW_PINS {2}
#define KEY_COL_PINS {3}

#define KEY_MAP                                                                \
  {                                                                            \
      {HID_KEY_A},                                                             \
  }

// OLED
#define OLED_SPI_PORT spi1
#define OLED_PIN_SCK 10
#define OLED_PIN_MOSI 11
#define OLED_PIN_DC 12
#define OLED_PIN_CS 13
#define OLED_PIN_RST 14

// WS2812
#define WS2812_PIN 16

#endif
