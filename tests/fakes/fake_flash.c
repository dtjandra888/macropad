#include "fake_flash.h"

#include "hardware/sync.h"

#include <string.h>

uint8_t fake_flash_memory[PICO_FLASH_SIZE_BYTES];

int fake_flash_erase_count;
int fake_flash_program_count;
uint32_t fake_flash_last_erase_offset;
size_t fake_flash_last_erase_size;
uint32_t fake_flash_last_program_offset;
size_t fake_flash_last_program_size;
bool fake_flash_interrupts_were_disabled;

static bool interrupts_disabled;

void fake_flash_reset(void) {
  memset(fake_flash_memory, 0xFF, sizeof(fake_flash_memory));
  fake_flash_erase_count = 0;
  fake_flash_program_count = 0;
  fake_flash_last_erase_offset = 0;
  fake_flash_last_erase_size = 0;
  fake_flash_last_program_offset = 0;
  fake_flash_last_program_size = 0;
  fake_flash_interrupts_were_disabled = true;
  interrupts_disabled = false;
}

uint32_t save_and_disable_interrupts(void) {
  interrupts_disabled = true;
  return 0x1234;
}

void restore_interrupts(uint32_t status) {
  (void)status;
  interrupts_disabled = false;
}

void flash_range_erase(uint32_t flash_offs, size_t count) {
  fake_flash_erase_count++;
  fake_flash_last_erase_offset = flash_offs;
  fake_flash_last_erase_size = count;
  fake_flash_interrupts_were_disabled &= interrupts_disabled;
  memset(fake_flash_memory + flash_offs, 0xFF, count);
}

void flash_range_program(uint32_t flash_offs, const uint8_t *data,
                         size_t count) {
  fake_flash_program_count++;
  fake_flash_last_program_offset = flash_offs;
  fake_flash_last_program_size = count;
  fake_flash_interrupts_were_disabled &= interrupts_disabled;
  // Programming can only clear bits, like real NOR flash
  for (size_t i = 0; i < count; i++) {
    fake_flash_memory[flash_offs + i] &= data[i];
  }
}
