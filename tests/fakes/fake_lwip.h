#ifndef FAKE_LWIP_H
#define FAKE_LWIP_H

#include "lwip/pbuf.h"

void fake_lwip_reset(void);

extern int fake_pbuf_free_count;
extern struct pbuf *fake_pbuf_last_freed;

#endif
