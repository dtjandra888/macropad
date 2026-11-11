#ifndef HARDWARE_SYNC_H_STUB
#define HARDWARE_SYNC_H_STUB

#include <stdint.h>

uint32_t save_and_disable_interrupts(void);
void restore_interrupts(uint32_t status);

#endif
