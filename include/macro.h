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

typedef struct {
    KeyStroke strokes[MACRO_MAX_STROKES];
    int length;
} Macro;

void macro_start(const Macro* macro);
void macro_task(void);
bool macro_running(void);

#endif
