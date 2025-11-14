#include <stdio.h>
#include <time.h>
#include "log.h"

void log_header(log_level_t level, char *module) {
  time_t rawtime;
  struct tm * timeinfo;
  char timestamp_buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  char *level_str;
  switch (level) {
    case LOG_LEVEL_INFO:
      level_str = "\033[34mINFO";
      break;
    case LOG_LEVEL_ERROR:
      level_str = "\033[31mERROR";
      break;
  }
  fprintf(stderr, "\033[90m[%s] %s\033[90m %s: \033[0m", timestamp_buffer, level_str, module);
}
