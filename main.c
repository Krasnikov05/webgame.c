#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "log.h"
#include "http.h"
#include "buffered_writer.h"
#include "json_writer.h"

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
  while (true) {
    char *buffer = "<title>Hello world!</title><h1>Hello world</h1>";
    accept_http_request(&http_server);
    if (!http_server.is_ok) {
      continue;
    }
    json_start(&json_writer);
    json_start_array(&json_writer);
    json_write_number(&json_writer, 32);
    json_write_number(&json_writer, 64);
    json_write_number(&json_writer, 128);
    json_write_string(&json_writer, "Hello\r\nWorld");
    json_stop_array(&json_writer);
    json_end(&json_writer);
  }
  return 0;
}
