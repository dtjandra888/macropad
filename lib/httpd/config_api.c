#include "config_api.h"

#include <stdio.h>

size_t config_to_json(char *buffer, size_t size)
{
    return snprintf(
        buffer,
        size,
        "{"
        "\"test\":true"
        "}"
    );
}
