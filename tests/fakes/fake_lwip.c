#include "fake_lwip.h"

#include <stddef.h>

int fake_pbuf_free_count;
struct pbuf *fake_pbuf_last_freed;

void fake_lwip_reset(void) {
  fake_pbuf_free_count = 0;
  fake_pbuf_last_freed = NULL;
}

u8_t pbuf_free(struct pbuf *p) {
  fake_pbuf_free_count++;
  fake_pbuf_last_freed = p;
  return 1;
}
