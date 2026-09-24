/// Logging through oled interface
/// Temp solution until web logger exists
/// or I figure out how to set up tinyusb logger

#include "logger.h"
#include "oled.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_LINES 8
#define LOG_LINE_LENGTH 21

static char log_buffer[LOG_LINES][LOG_LINE_LENGTH];

static void add_log(const char *prefix, const char *format, va_list args) {
  char message[LOG_LINE_LENGTH + 1];

  vsnprintf(message, sizeof(message), format, args);

  // Shift existing lines upward
  for (int i = 0; i < LOG_LINES - 1; i++) {
    strcpy(log_buffer[i], log_buffer[i + 1]);
  }

  // Add new message
  snprintf(log_buffer[LOG_LINES - 1], sizeof(log_buffer[LOG_LINES - 1]), "%s%s",
           prefix, message);

  // Redraw OLED
  oled_clear();

  for (int i = 0; i < LOG_LINES; i++) {
    oled_draw_string(0, i * 8, log_buffer[i]);
  }

  oled_update();
}

void logger_init(void) { logger_clear(); }

void logger_clear(void) {
  for (int i = 0; i < LOG_LINES; i++) {
    log_buffer[i][0] = '\0';
  }
  oled_clear();
}

void log_info(const char *format, ...) {
  va_list args;
  va_start(args, format);

  add_log("", format, args);

  va_end(args);
}

void log_error(const char *format, ...) {
  va_list args;
  va_start(args, format);

  add_log("ERR: ", format, args);

  va_end(args);
}
