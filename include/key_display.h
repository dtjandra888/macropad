#ifndef KEY_DISPLAY_H
#define KEY_DISPLAY_H

/// Shows one tile per key on the OLED: outlined when released, filled while
/// pressed. Owns the display in keyboard mode.

void key_display_init(void);

// Redraws only when a key's debounced state has changed
void key_display_task(void);

#endif
