#ifndef MACRO_H__
#define MACRO_H__
// Contains type definitions relevant to macro logic

#include <stdint.h>
#include <stdbool.h>

#define MACRO_MAX_STROKES 5

typedef struct {
    uint8_t key;
    uint8_t modifier;
} KeyStroke;

typedef enum {
    // Play the strokes in order; repeats while the key is held
    MACRO_MODE_SEQUENCE = 0,
    // Hold strokes[0] down for as long as the physical key is held
    MACRO_MODE_HOLD = 1,
} MacroMode;

typedef struct {
    uint8_t length;
    uint8_t mode; // MacroMode; HOLD requires length == 1
    KeyStroke strokes[MACRO_MAX_STROKES];
} Macro;

// Returns true if the macro was started, false if it was empty or another
// macro is still running.
bool macro_start(const Macro* macro);
void macro_task(void);
bool macro_running(void);

#endif
