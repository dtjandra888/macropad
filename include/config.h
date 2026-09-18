#ifndef CONFIG_H_
#define CONFIG_H_

/// Defining functions for flashing/loading configs and converting to json for web server

#include "macro.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t macro_count;
    Macro macros[3];        // TODO: should not be hardcoded to support different PCBs/Key configs
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

void save_config(const ConfigStorage* config);
void load_config(ConfigStorage* config);
bool config_is_valid(const ConfigStorage* config);


#endif
