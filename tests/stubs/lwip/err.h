// Test stand-in for lwIP's err.h and basic types.
#ifndef LWIP_ERR_H_STUB
#define LWIP_ERR_H_STUB

#include <stdint.h>

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef int8_t err_t;

#define ERR_OK 0
#define ERR_VAL -6
#define ERR_ARG -16

#endif
