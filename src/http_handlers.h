#ifndef HTTP_HANDLERS_H
#define HTTP_HANDLERS_H

#include <stdbool.h>
#include "http.h"
#include "json_writer.h"
#include "sessions.h"

#define STATIC_PATH_PREFIX "/static/"
#define STATIC_FILES_DIRECTORY "./static/"

typedef struct {
  char *name;
  int size;
  char *data;
  void *next;
} static_file_entry_t;

typedef static_file_entry_t *static_handler_t;

void init_static_handler(static_handler_t *static_handler);

bool starts_with(char *string, char *prefix);

bool ends_with(char *string, char *suffix);

bool match_static_path(http_server_t *http_server);

char *get_mime(char *path);

void send_static_file(static_handler_t *static_handler, http_server_t *http_server, char *name);

void handle_static_request(static_handler_t *static_handler, http_server_t *http_server);

void handle_session_list_request(session_manager_t *session_manager, json_writer_t *json_writer);

void handle_auth_request(
  http_server_t *http_server,
  session_manager_t *session_manager,
  static_handler_t *static_handler
);

#endif
