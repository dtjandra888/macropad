

#include "config.h"

#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

#include <stdbool.h>
#include <string.h>

// using final 4096 bytes of board
// if firmware size gets close to 2 Mb this may not work
#define CONFIG_FLASH_SIZE FLASH_SECTOR_SIZE
#define CONFIG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - CONFIG_FLASH_SIZE)

#define CONFIG_MAGIC 0x4D414352u // "MACR"

bool config_save(void) {}

bool config_load(void) {
  // XIP base address is the starting memory location in a processor's address
  // map where an Execute in place external storage device or flash memory is
  // mapped
  //

}

static uint32_t crc32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xFFFFFFFFu;

  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];

    for (int bit = 0; bit < 8; ++bit) {
      if (crc & 1u) {
        crc = (crc >> 1) ^ 0xEDB88320u;
      } else {
        crc >>= 1;
      }
    }
  }

  return ~crc;
}

static void flash_erase_callback(void *param) {
  (void)param;

  flash_range_erase(CONFIG_FLASH_OFFSET, CONFIG_FLASH_SIZE);
}

static void flash_program_callback(void *param) {
  const uint8_t *buffer = param;

  flash_range_program(CONFIG_FLASH_OFFSET, buffer, CONFIG_FLASH_SIZE);
}
