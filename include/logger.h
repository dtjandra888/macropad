#ifndef LOGGER_H
#define LOGGER_H

/// Logging over Oled display

void logger_init(void);
void logger_clear(void);

void log_info(const char *format, ...);
void log_error(const char *format, ...);

#endif
