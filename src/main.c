#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "log.h"
#include "http.h"
#include "buffered_writer.h"
#include "json_writer.h"
#include "http_handlers.h"

#define DEFAULT_HTTP_SERVER_PORT 8080

void print_usage(char *name) {
  printf(
    "Usage: %s [port]\n"
    "\n"
    "Options:\n"
    "  -h, --help\tPrint this message\n",
    name
  );
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  http_server_t http_server;
  buffered_writer_t buffered_writer;
  json_writer_t json_writer;
  static_handler_t static_handler;
  int port;
  if (argc == 1) {
    port = DEFAULT_HTTP_SERVER_PORT;
  } else if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      print_usage(argv[0]);
      exit(EXIT_SUCCESS);
    }
    port = atoi(argv[1]);
  } else {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  log_message(LOG_LEVEL_INFO, "main", "Starting...");
  init_http_server(&http_server, port);
  init_buffered_writer(&buffered_writer, &http_server);
  init_json_writer(&json_writer, &http_server, &buffered_writer);
  init_static_handler(&static_handler);
  while (true) {
    accept_http_request(&http_server);
    if (!http_server.is_ok) {
      continue;
    }
    if (match_static_path(&http_server)) {
      handle_static_request(&static_handler, &http_server);
      continue;
    }
    if (strcmp(http_server.path, "/") == 0) {
      send_static_file(&static_handler, &http_server, "index.html");
      continue;
    }
    send_simple_http_error(&http_server, HTTP_STATUS_NOT_FOUND);
  }
  return 0;
}
