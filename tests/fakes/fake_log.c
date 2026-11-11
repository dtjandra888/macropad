// Logging goes to the OLED on the device; tests drop it.
#include "logger.h"

void logger_init(void) {}
void logger_clear(void) {}
void log_info(const char *format, ...) { (void)format; }
void log_error(const char *format, ...) { (void)format; }
