#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "http.h"
#include "arena.h"
#include "log.h"

void init_http_server(http_server_t *http_server, int port) {
  init_arena(&http_server->arena, HTTP_ARENA_SIZE);
  http_server->connected_client_count = 0;
  http_server->pfds = calloc(HTTP_SERVER_MAX_CONNECTED + 1, sizeof(struct pollfd));
  if (http_server->pfds == NULL) {
    log_message(LOG_LEVEL_ERROR, "http", "Failed allocating pfds");
    exit(EXIT_FAILURE);
  }
  http_server->buffer = (char *)malloc(HTTP_REQUEST_BUFFER_SIZE);
  if (http_server->buffer == NULL) {
    log_message(LOG_LEVEL_ERROR, "http", "Failed allocating request buffer");
    exit(EXIT_FAILURE);
  }
  int server_fd;
  struct sockaddr_in server_addr;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "setsockopt: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  server_addr.sin_family = AF_INET;        
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "bind: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, HTTP_SERVER_BACKLOG) < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "listen: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  socklen_t len = sizeof(server_addr);
  if (getsockname(server_fd, (struct sockaddr *)&server_addr, &len) < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "getsockname: %s", strerror(errno));
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  port = ntohs(server_addr.sin_port);
  http_server->server_fd = server_fd;
  http_server->pfds[0].fd = server_fd;
  http_server->pfds[0].events = POLLIN;
  log_message(LOG_LEVEL_INFO, "http", "Server started on port %d", port);
}

void send_http_head(http_server_t *http_server, char *status) {
  char buffer[HTTP_SERVER_RESPONSE_LINE_SIZE];
  snprintf(buffer, HTTP_SERVER_RESPONSE_LINE_SIZE, "HTTP/1.1 %s\r\n", status);
  write(http_server->client_fd, buffer, strlen(buffer));
}

void send_http_header(http_server_t *http_server, char *key, char *value) {
  char buffer[HTTP_SERVER_RESPONSE_LINE_SIZE];
  snprintf(buffer, HTTP_SERVER_RESPONSE_LINE_SIZE, "%s: %s\r\n", key, value);
  write(http_server->client_fd, buffer, strlen(buffer));
}

void send_default_http_headers(http_server_t *http_server) {
  send_http_header(http_server, "Transfer-Encoding", "chunked");
}

void send_http_end_headers(http_server_t *http_server) {
  write(http_server->client_fd, "\r\n", 2);
}

void send_http_content(http_server_t *http_server, char *buffer, int size) {
  char size_buffer[64];
  snprintf(size_buffer, sizeof(size_buffer), "%X\r\n", size);
  write(http_server->client_fd, size_buffer, strlen(size_buffer));
  write(http_server->client_fd, buffer, size);
  write(http_server->client_fd, "\r\n", 2);
}

void close_http_connection(http_server_t *http_server) {
  for (int i = 0; i < http_server->connected_client_count; i++) {
    if (http_server->pfds[i + 1].fd == http_server->client_fd) {
      log_message(LOG_LEVEL_INFO, "http", "Removing FD %d", http_server->client_fd);
      http_server->connected_client_count--;
      http_server->pfds[i + 1].fd = http_server->pfds[http_server->connected_client_count + 1].fd;
      break;
    }
  }
  write(http_server->client_fd, "0\r\n\r\n", 5);
  close(http_server->client_fd);
}

void send_simple_http_error(http_server_t *http_server, char *status) {
  char buffer[HTTP_SERVER_RESPONSE_LINE_SIZE];
  snprintf(buffer, HTTP_SERVER_RESPONSE_LINE_SIZE, "<h1>%s</h1>", status);
  send_http_head(http_server, status);
  send_default_http_headers(http_server);
  send_http_header(http_server, "Content-Type", "text/html");
  send_http_end_headers(http_server);
  send_http_content(http_server, buffer, strlen(buffer));
  close_http_connection(http_server);
}

char read_url_hex(char *buffer) {
  char out = 0;
  for (int i = 1; i < 3; i++) {
    out *= 16;
    switch (buffer[i]) {
      case '0'...'9':
        out += buffer[i] - '0';
      break;
      case 'a'...'f':
        out += buffer[i] - 'a' + 10;
      break;
    }
  }
  return out;
}

char *read_url_segment(http_server_t *http_server, int *index) {
  char *delimiter = "\r\n?&= ";
  int end;
  int length = 0;
  for (int i = *index; i < HTTP_REQUEST_BUFFER_SIZE; i++) {
    bool end_found = false;
    for (int j = 0; delimiter[j] != '\0'; j++) {
      if (http_server->buffer[i] == delimiter[j]) {
        end_found = true;
        break;
      }
    }
    if (end_found || (http_server->buffer[i] == '\0')) {
      end = i;
      break;
    }
    if (http_server->buffer[i] == '%') {
      i += 2;
    }
    length++;
  }
  char *result = alloc_arena(&http_server->arena, length + 1);
  if (result == NULL) {
    return NULL;
  }
  result[length] = '\0';
  for (int i = *index, j = 0; j < length; i++, j++) {
    if (http_server->buffer[i] == '+') {
      result[j] = ' ';
    } else if (http_server->buffer[i] == '%') {
      result[j] = read_url_hex(&http_server->buffer[i]);
      i += 2;
    } else {
      result[j] = http_server->buffer[i];
    }
  }
  // log_message(LOG_LEVEL_INFO, "http", "Got url segment '%s'", result);
  *index = end;
  return result;
}

bool parse_query_entry(http_server_t *http_server, int *index) {
  char *key = read_url_segment(http_server, index);
  if (key == NULL) {
    return false;
  }
  if (http_server->buffer[*index] != '=') {
    return false;
  }
  (*index)++;
  char *value = read_url_segment(http_server, index);
  if (value == NULL) {
    return false;
  }
  parameter_list_entry_t **tail = &http_server->parameter_list;
  while (*tail != NULL) {
    tail = (parameter_list_entry_t **)&((**tail).next);
  }
  parameter_list_entry_t *new_entry = (parameter_list_entry_t *)alloc_arena(
    &http_server->arena,
    sizeof(parameter_list_entry_t)
  );
  if (new_entry == NULL) {
    return false;
  }
  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = NULL;
  *tail = new_entry;
  log_message(LOG_LEVEL_INFO, "http", "Got parameter '%s' = '%s'", key, value);
  return true;
}

void parse_request(http_server_t *http_server) {
  free_arena(&http_server->arena);
  http_server->parameter_list = NULL;
  int i = 0;
  char *prefix = "GET ";
  while(prefix[i]) {
    if (http_server->buffer[i] != prefix[i]) {
      log_message(LOG_LEVEL_ERROR, "http", "Request doesn't start with 'GET '");
      send_simple_http_error(http_server, HTTP_STATUS_NOT_IMPLEMENTED);
      return;
    }
    i++;
  }
  http_server->path = read_url_segment(http_server, &i);
  log_message(LOG_LEVEL_INFO, "http", "Got path = '%s'", http_server->path);
  if (http_server->buffer[i] == '?') {
    i++;
    if (!parse_query_entry(http_server, &i)) {
      log_message(LOG_LEVEL_ERROR, "http", "Error while reading request url");
      send_simple_http_error(http_server, HTTP_STATUS_BAD_REQUEST);
      return;
    }
    while (http_server->buffer[i] == '&') {
      i++;
      if (!parse_query_entry(http_server, &i)) {
        log_message(LOG_LEVEL_ERROR, "http", "Error while reading request url");
        send_simple_http_error(http_server, HTTP_STATUS_BAD_REQUEST);
        return;
      }
    }
  }
  http_server->is_ok = true;
}

void accept_http_request(http_server_t *http_server) {
  http_server->is_ok = false;
  int ready = poll(http_server->pfds, http_server->connected_client_count + 1, -1);
  if (ready < 0) {
    log_message(LOG_LEVEL_ERROR, "http", "poll: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  int count = http_server->connected_client_count;
  if (
    (http_server->connected_client_count < HTTP_SERVER_MAX_CONNECTED)
    && ((http_server->pfds[0].revents & POLLIN) != 0)
  ) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    client_fd = accept(http_server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
      log_message(LOG_LEVEL_ERROR, "http", "accept: %s", strerror(errno));
      return;
    }
    char *host = inet_ntoa(client_addr.sin_addr);
    int port = ntohs(client_addr.sin_port);
    log_message(LOG_LEVEL_INFO, "http", "Connected client %s:%d", host, port);
    http_server->pfds[http_server->connected_client_count + 1].fd = client_fd;
    http_server->pfds[http_server->connected_client_count + 1].events = POLLIN;
    http_server->connected_client_count++;
    log_message(LOG_LEVEL_INFO, "http", "Connected clients count %d", http_server->connected_client_count);
  }
  for (int i = 0; i < count; i++) {
    if ((http_server->pfds[i + 1].revents & POLLIN) != 0) {
      int client_fd = http_server->pfds[i + 1].fd;
      int n = read(client_fd, http_server->buffer, HTTP_REQUEST_BUFFER_SIZE);
      if (n > 0) {
        http_server->buffer[n] = '\0';
      }
      http_server->client_fd = client_fd;
      parse_request(http_server);
      return;
    }
  }
}
