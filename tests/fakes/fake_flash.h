#ifndef FAKE_FLASH_H
#define FAKE_FLASH_H

#include "hardware/flash.h"

#include <stdbool.h>

void fake_flash_reset(void); // fills flash with 0xFF (erased)

extern int fake_flash_erase_count;
extern int fake_flash_program_count;
extern uint32_t fake_flash_last_erase_offset;
extern size_t fake_flash_last_erase_size;
extern uint32_t fake_flash_last_program_offset;
extern size_t fake_flash_last_program_size;
// True while flash calls happen with interrupts disabled
extern bool fake_flash_interrupts_were_disabled;

#endif
