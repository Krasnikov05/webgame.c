#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <poll.h>
#include "arena.h"

#define HTTP_SERVER_BACKLOG 16
#define HTTP_SERVER_MAX_CONNECTED 16
#define HTTP_ARENA_SIZE 4096
#define HTTP_REQUEST_BUFFER_SIZE 4096
#define HTTP_SERVER_RESPONSE_LINE_SIZE 512
#define HTTP_STATUS_OK "200 OK"
#define HTTP_STATUS_TEMPORARY_REDIRECT "307 Temporary Redirect"
#define HTTP_STATUS_BAD_REQUEST "400 Bad Request"
#define HTTP_STATUS_NOT_FOUND "404 Not Found"
#define HTTP_STATUS_INTERNAL_SERVER_ERROR "500 Internal Server Error"
#define HTTP_STATUS_NOT_IMPLEMENTED "501 Not Implemented"

typedef struct {
  char *key;
  char *value;
  void *next;
} parameter_list_entry_t;

typedef parameter_list_entry_t *parameter_list_t;

typedef struct {
  int server_fd;
  int client_fd;
  bool is_ok;
  char *buffer;
  arena_t arena;
  char *path;
  parameter_list_t parameter_list;
  int connected_client_count;
  struct pollfd *pfds;
} http_server_t;

void init_http_server(http_server_t *http_server, int port);

void send_http_head(http_server_t *http_server, char *status);

void send_http_header(http_server_t *http_server, char *key, char *value);

void send_default_http_headers(http_server_t *http_server);

void send_http_end_headers(http_server_t *http_server);

void send_http_content(http_server_t *http_server, char *buffer, int size);

void send_simple_http_error(http_server_t *http_server, char *status);

void send_http_redirect(http_server_t *http_server, char *path);

void close_http_connection(http_server_t *http_server);

void accept_http_request(http_server_t *http_server);

char *get_http_parameter(http_server_t *http_server, char *key);

#endif
