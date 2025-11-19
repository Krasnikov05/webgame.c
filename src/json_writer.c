#include <stdio.h>
#include <string.h>
#include "json_writer.h"
#include "buffered_writer.h"

void init_json_writer(
  json_writer_t *json_writer, http_server_t *http_server, buffered_writer_t *buffered_writer
) {
  json_writer->http_server = http_server;
  json_writer->buffered_writer = buffered_writer;
}

void json_start_dict(json_writer_t *json_writer) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  json_writer->is_first = true;
  buffered_writer_write(json_writer->buffered_writer, "{", 1);
}

void json_stop_dict(json_writer_t *json_writer) {
  buffered_writer_write(json_writer->buffered_writer, "}", 1);
  json_writer->is_first = false;
}

void json_start_array(json_writer_t *json_writer) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  json_writer->is_first = true;
  buffered_writer_write(json_writer->buffered_writer, "[", 1);
}

void json_stop_array(json_writer_t *json_writer) {
  json_writer->is_first = false;
  buffered_writer_write(json_writer->buffered_writer, "]", 1);
}

void json_write_number(json_writer_t *json_writer, double number) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  json_writer->is_first = false;
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%f", number);
  buffered_writer_write(json_writer->buffered_writer, buffer, strlen(buffer));
}

void json_write_bool(json_writer_t *json_writer, bool value) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  json_writer->is_first = false;
  char *buffer = value ? "true" : "false";
  buffered_writer_write(json_writer->buffered_writer, buffer, strlen(buffer));
}

bool is_printable(char c) {
  switch (c) {
    case '\0':
    case '\r':
    case '\n':
    case '"':
    case '\t':
      return false;
  }
  return true;
}

char *escape(char c) {
  switch (c) {
    case '\r':
      return "\\r";
    case '\n':
      return "\\n";
    case '"':
      return "\\\"";
    case '\t':
      return "\\t";
  }
  return NULL;
}

void json_write_string(json_writer_t *json_writer, char *string) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  buffered_writer_write(json_writer->buffered_writer, "\"", 1);
  json_writer->is_first = false;
  int start = 0, end = 0;
  while (string[start] != '\0') {
    while (is_printable(string[end])) {
      end++;
    }
    buffered_writer_write(json_writer->buffered_writer, string + start, end - start);
    char *escaped = escape(string[end]);
    if (escaped == NULL) {
      break;
    }
    buffered_writer_write(json_writer->buffered_writer, escaped, 2);
    start = ++end;
  }
  buffered_writer_write(json_writer->buffered_writer, "\"", 1);
}

void json_write_key(json_writer_t *json_writer, char *key) {
  json_write_string(json_writer, key);
  json_writer->is_first = true;
  buffered_writer_write(json_writer->buffered_writer, ":", 1);
}

void json_write_null(json_writer_t *json_writer) {
  if (!json_writer->is_first) {
    buffered_writer_write(json_writer->buffered_writer, ",", 1);
  }
  json_writer->is_first = false;
  char *buffer = "null";
  buffered_writer_write(json_writer->buffered_writer, buffer, strlen(buffer));
}

void json_start(json_writer_t *json_writer) {
  json_writer->is_first = true;
  send_http_head(json_writer->http_server, HTTP_STATUS_OK);
  send_default_http_headers(json_writer->http_server);
  send_http_header(json_writer->http_server, "Content-Type", "application/json");
  send_http_end_headers(json_writer->http_server);
}

void json_end(json_writer_t *json_writer) {
  json_writer->is_first = true;
  buffered_writer_end(json_writer->buffered_writer);
}
