#include "config.h"

#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

#include <hardware/sync.h>
#include <stdbool.h>
#include <string.h>

// using final 4096 bytes of board
// if firmware size gets close to 2 Mb this may not work
#define CONFIG_FLASH_SIZE FLASH_SECTOR_SIZE
#define CONFIG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - CONFIG_FLASH_SIZE)

#define CONFIG_MAGIC 0x4D414352u // "MACR"

void save_config(const ConfigStorage* config) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0xFF, sizeof(buffer));
    memcpy(buffer, config, sizeof(ConfigStorage));

    // Disable XIP access during flash operations
    uint32_t inter = save_and_disable_interrupts();
    flash_range_erase(CONFIG_FLASH_OFFSET, CONFIG_FLASH_SIZE);
    flash_range_program(CONFIG_FLASH_OFFSET, buffer, FLASH_PAGE_SIZE);

    restore_interrupts(inter);
}

void load_config(ConfigStorage* config) {
  // XIP base address is the starting memory location in a processor's address
  // map where an Execute in place external storage device or flash memory is
  // mapped
  const ConfigStorage *stored =
      (const ConfigStorage *)(XIP_BASE + CONFIG_FLASH_OFFSET);
  memcpy(config, stored, sizeof(ConfigStorage));
}

// I have no idea how this works but I copied off wikipedia
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

bool config_is_valid(const ConfigStorage *config) {
    if (config->header.magic != CONFIG_MAGIC) {
        return false;
    }

    if (config->header.size != sizeof(Config)) {
        return false;
    }

    uint32_t crc = crc32(
        (const uint8_t *)&config->config,
        sizeof(Config)
    );

    return crc == config->header.crc32;
}
