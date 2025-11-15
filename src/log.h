#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef enum {
  LOG_LEVEL_INFO,
  LOG_LEVEL_ERROR,
} log_level_t;

void log_header(log_level_t level, char *module);

#define log_message(level, module, fmt, ...) if (1) { \
  log_header(level, module); fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
}

#endif
