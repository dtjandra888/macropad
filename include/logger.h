#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

/// Logging over Oled display

void logger_init(void);
void logger_clear(void);

// When disabled, messages are still buffered but not drawn, so another
// module can own the display. Re-enabling redraws the buffered lines.
void logger_set_display(bool enabled);

void log_info(const char *format, ...);
void log_error(const char *format, ...);

#endif
