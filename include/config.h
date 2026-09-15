/// Defining functions for flashing/loading configs and converting to json for web server
///

#ifndef CONFIG_H_
#define CONFIG_H_

#include "macro.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t macro_count;
    Macro macros[3];        // TODO: should not be hardcoded to support different PCBs/Key configs
    uint32_t crc32;
} Config;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t crc32;
} ConfigHeader;

typedef struct {
    ConfigHeader header;
    Config config;
} ConfigStorage;

bool config_save(void);
bool config_load(void);

#endif
