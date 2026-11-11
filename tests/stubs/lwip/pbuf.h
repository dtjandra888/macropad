// Test stand-in for lwIP's pbuf.h: just the fields our code reads.
#ifndef LWIP_PBUF_H_STUB
#define LWIP_PBUF_H_STUB

#include "lwip/err.h"

struct pbuf {
  struct pbuf *next;
  void *payload;
  u16_t tot_len;
  u16_t len;
};

u8_t pbuf_free(struct pbuf *p);

#endif
