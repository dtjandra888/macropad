#ifndef CONFIG_H_
#define CONFIG_H_

/// Defining functions for flashing/loading configs and converting to json for
/// web server

#include "macro.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// using final 4096 bytes of board
// if firmware size gets close to 2 Mb this may not work
#define CONFIG_FLASH_SIZE FLASH_SECTOR_SIZE
#define CONFIG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - CONFIG_FLASH_SIZE)

#define CONFIG_MAGIC 0x4D414352u // "MACR"

typedef struct {
  uint16_t macro_count;
  Macro macros[3]; // TODO: should not be hardcoded to support different
                   // PCBs/Key configs
} Config;

typedef struct {
  uint32_t magic;
  uint16_t size;
  uint32_t crc32;
} ConfigHeader;

typedef struct {
  ConfigHeader header;
  Config config;
} ConfigStorage;

extern Config config;

void save_config(const ConfigStorage *config);
void load_config(ConfigStorage *config);
bool config_is_valid(const ConfigStorage *config);
uint32_t crc32(const uint8_t *data, size_t length);

#endif
