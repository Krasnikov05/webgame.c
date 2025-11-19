#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <stdbool.h>
#include "http.h"
#include "buffered_writer.h"

typedef struct {
  http_server_t *http_server;
  buffered_writer_t *buffered_writer;
  bool is_first;
} json_writer_t;

void init_json_writer(
  json_writer_t *json_writer, http_server_t *http_server, buffered_writer_t *buffered_writer
);

void json_start_dict(json_writer_t *json_writer);

void json_stop_dict(json_writer_t *json_writer);

void json_start_array(json_writer_t *json_writer);

void json_stop_array(json_writer_t *json_writer);

void json_write_number(json_writer_t *json_writer, double number);

void json_write_bool(json_writer_t *json_writer, bool value);

void json_write_string(json_writer_t *json_writer, char *string);

void json_write_key(json_writer_t *json_writer, char *key);

void json_write_null(json_writer_t *json_writer);

void json_start(json_writer_t *json_writer);

void json_end(json_writer_t *json_writer);

#endif
