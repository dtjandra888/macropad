#include "key_display.h"

#include "key.h"
#include "macropad_config.h"
#include "oled.h"

#include <stdbool.h>

#define SCREEN_W 128
#define SCREEN_H 64

#define KEY_COUNT (KEY_ROWS * KEY_COLS)
#define TILE_SIZE 36
#define TILE_GAP 8

// Tiles sit in one row, centred on the screen
#define ROW_W (KEY_COUNT * TILE_SIZE + (KEY_COUNT - 1) * TILE_GAP)
#define TILE_X0 ((SCREEN_W - ROW_W) / 2)
#define TILE_Y ((SCREEN_H - TILE_SIZE) / 2)

_Static_assert(ROW_W <= SCREEN_W, "key tiles do not fit on the OLED");

static bool drawn_state[KEY_COUNT];

static void draw(void) {
  oled_clear_buffer();

  for (int i = 0; i < KEY_COUNT; i++) {
    int x = TILE_X0 + i * (TILE_SIZE + TILE_GAP);
    oled_draw_rect(x, TILE_Y, TILE_SIZE, TILE_SIZE, drawn_state[i]);
  }

  // Full frame is ~8 ms over SPI, so only push when something changed
  oled_update();
}

void key_display_init(void) {
  for (int i = 0; i < KEY_COUNT; i++) {
    drawn_state[i] = key_is_pressed(i);
  }
  draw();
}

void key_display_task(void) {
  bool changed = false;

  for (int i = 0; i < KEY_COUNT; i++) {
    bool pressed = key_is_pressed(i);
    if (pressed != drawn_state[i]) {
      drawn_state[i] = pressed;
      changed = true;
    }
  }

  if (changed){
    draw();
  }
}
