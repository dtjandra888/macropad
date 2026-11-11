// Test stand-in for hardware/flash.h. Flash is a RAM array (fakes/fake_flash.c)
// and XIP_BASE points at it, so load_config reads what save_config wrote.
#ifndef HARDWARE_FLASH_H_STUB
#define HARDWARE_FLASH_H_STUB

#include <stddef.h>
#include <stdint.h>

#define FLASH_PAGE_SIZE 256u
#define FLASH_SECTOR_SIZE 4096u
#define PICO_FLASH_SIZE_BYTES (2u * FLASH_SECTOR_SIZE)

extern uint8_t fake_flash_memory[PICO_FLASH_SIZE_BYTES];
#define XIP_BASE ((uintptr_t)fake_flash_memory)

void flash_range_erase(uint32_t flash_offs, size_t count);
void flash_range_program(uint32_t flash_offs, const uint8_t *data,
                         size_t count);

#endif
