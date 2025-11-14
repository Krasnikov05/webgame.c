#ifndef BUFFERED_WRITER_H
#define BUFFERED_WRITER_H

#include "http.h"

#define BUFFERED_WRITER_BUFFER_SIZE 4096

typedef struct {
  http_server_t *http_server;
  char *buffer;
  int position;
} buffered_writer_t;

void init_buffered_writer(buffered_writer_t *buffered_writer, http_server_t *http_server);

void buffered_writer_flush(buffered_writer_t *buffered_writer);

void buffered_writer_write(buffered_writer_t *buffered_writer, char *buffer, int size);

void buffered_writer_end(buffered_writer_t *buffered_writer);

#endif
