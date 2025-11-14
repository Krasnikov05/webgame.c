#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "log.h"
#include "http.h"

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
  while (true) {
    char *buffer = "<title>Hello world!</title><h1>Hello world</h1>";
    accept_http_request(&http_server);
    if (!http_server.is_ok) {
      continue;
    }
    send_http_head(&http_server, HTTP_STATUS_OK);
    send_default_http_headers(&http_server);
    send_http_header(&http_server, "Content-Type", "text/html");
    send_http_end_headers(&http_server);
    send_http_content(&http_server, buffer, strlen(buffer));
    close_http_connection(&http_server);
  }
  return 0;
}
