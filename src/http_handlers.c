#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "http_handlers.h"
#include "http.h"
#include "log.h"
#include "json_writer.h"
#include "sessions.h"

void init_static_handler(static_handler_t *static_handler) {
  DIR *d = opendir(STATIC_FILES_DIRECTORY);
  if (!d) {
    log_message(LOG_LEVEL_ERROR, "http_handlers", "Can't open %s directory", STATIC_FILES_DIRECTORY);
    exit(EXIT_FAILURE);
  }
  struct dirent *dir;
  FILE *file;
  static_file_entry_t *entry;
  char path[256];
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type != DT_REG) {
      continue;
    }
    entry = (static_file_entry_t *)malloc(sizeof(static_file_entry_t));
    if (entry == NULL) {
      log_message(LOG_LEVEL_ERROR, "http_handlers", "Failed allocating static_file_entry_t");
      exit(EXIT_FAILURE);
    }
    entry->name = strdup(dir->d_name);
    entry->next = NULL;
    if (entry->name == NULL) {
      log_message(LOG_LEVEL_ERROR, "http_handlers", "Failed allocating static_file_entry_t name");
      exit(EXIT_FAILURE);
    }
    snprintf(path, sizeof(path), "%s/%s", STATIC_FILES_DIRECTORY, entry->name);
    file = fopen(path, "rb");
    if (file == NULL) {
      log_message(LOG_LEVEL_ERROR, "http_handlers", "Can't open file %s", path);
      exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    entry->size = ftell(file);
    fseek(file, 0, SEEK_SET);
    entry->data = (char *)malloc(entry->size);
    if (entry->data == NULL) {
      log_message(LOG_LEVEL_ERROR, "http_handlers", "Can't allocate %d bytes for %s", entry->size, entry->name);
      exit(EXIT_FAILURE);
    }
    fread(entry->data, 1, entry->size, file);
    fclose(file);
    *static_handler = entry;
    static_handler = (static_file_entry_t **)&entry->next;
    log_message(LOG_LEVEL_INFO, "http_handlers", "Loaded static file %s", dir->d_name);
  }
  closedir(d);
}

bool starts_with(char *string, char *prefix) {
    if (string == NULL || prefix == NULL) {
      return false;
    }
    int len_string = strlen(string);
    int len_prefix = strlen(prefix);
    if (len_prefix > len_string) {
      return false;
    }
    return strncmp(string, prefix, len_prefix) == 0;
}

bool ends_with(char *string, char *suffix) {
    if (string == NULL || suffix == NULL) {
      return false;
    }
    int len_string = strlen(string);
    int len_suffix = strlen(suffix);
    if (len_suffix > len_string) {
      return false;
    }
    return strncmp(string + (len_string - len_suffix), suffix, len_suffix) == 0;
}

bool match_static_path(http_server_t *http_server) {
  return starts_with(http_server->path, STATIC_PATH_PREFIX);
}

char *get_mime(char *path) {
  if (ends_with(path, ".html")) {
    return "text/html";
  }
  if (ends_with(path, ".js")) {
    return "text/javascript";
  }
  if (ends_with(path, ".css")) {
    return "text/css";
  }
  return "text/plain";
}

void send_static_file(static_handler_t *static_handler, http_server_t *http_server, char *name) {
  static_file_entry_t *entry = *static_handler;
  while (true) {
    if (entry == NULL) {
      send_simple_http_error(http_server, HTTP_STATUS_NOT_FOUND);
      return;
    }
    if (strcmp(entry->name, name) == 0) {
      break;
    }
    entry = entry->next;
  }
  send_http_head(http_server, HTTP_STATUS_OK);
  send_default_http_headers(http_server);
  send_http_header(http_server, "Content-Type", get_mime(name));
  send_http_end_headers(http_server);
  send_http_content(http_server, entry->data, entry->size);
  close_http_connection(http_server);
}

void handle_static_request(static_handler_t *static_handler, http_server_t *http_server) {
  char *name = http_server->path + strlen(STATIC_PATH_PREFIX);
  send_static_file(static_handler, http_server, name);
}

void handle_session_list_request(session_manager_t *session_manager, json_writer_t *json_writer) {
  clean_sessions(session_manager);
  json_start(json_writer);
  json_start_array(json_writer);
  session_t *session;
  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
    session = &session_manager->sessions[i];
    if (!session->is_active) {
      continue;
    }
    json_start_dict(json_writer);
    json_write_key(json_writer, "id");
    json_write_number(json_writer, session->id);
    json_write_key(json_writer, "name");
    json_write_string(json_writer, session->username);
    json_stop_dict(json_writer);
  }
  json_stop_array(json_writer);
  json_end(json_writer);
}

void handle_auth_request(
  http_server_t *http_server,
  session_manager_t *session_manager,
  static_handler_t *static_handler
) {
  char *username = get_http_parameter(http_server, "username");
  if (username == NULL) {
    send_static_file(static_handler, http_server, "auth.html");
    return;
  }
  if (!validate_username(username)) {
    send_simple_http_error(http_server, HTTP_STATUS_BAD_REQUEST);
    return;
  }
  session_t *session = new_session(session_manager, username);
  if (session == NULL) {
    send_simple_http_error(http_server, HTTP_STATUS_INTERNAL_SERVER_ERROR);
    return;
  }
  char path[256];
  snprintf(path, sizeof(path), "/?id=%d", session->id);
  send_http_redirect(http_server, path);
  log_message(LOG_LEVEL_INFO, "http_handlers", "Created new session #%d '%s'", session->id, session->username);
}
