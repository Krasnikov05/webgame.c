#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffered_writer.h"
#include "http.h"
#include "log.h"

void init_buffered_writer(buffered_writer_t *buffered_writer, http_server_t *http_server) {
  buffered_writer->position = 0;
  buffered_writer->http_server = http_server;
  buffered_writer->buffer = malloc(BUFFERED_WRITER_BUFFER_SIZE);
  if (buffered_writer->buffer == NULL) {
    log_message(
      LOG_LEVEL_ERROR,
      "buffered_writer",
      "Failed allocating %d bytes for buffered_writer",
      BUFFERED_WRITER_BUFFER_SIZE
    );
    exit(EXIT_FAILURE);
  }
  log_message(
    LOG_LEVEL_INFO,
    "buffered_writer",
    "Allocated %d bytes for buffered_writer",
    BUFFERED_WRITER_BUFFER_SIZE
  );
}

void buffered_writer_flush(buffered_writer_t *buffered_writer) {
  send_http_content(buffered_writer->http_server, buffered_writer->buffer, buffered_writer->position);
  buffered_writer->position = 0;
}

void buffered_writer_write(buffered_writer_t *buffered_writer, char *buffer, int size) {
  while (size > 0) {
    int available_space = BUFFERED_WRITER_BUFFER_SIZE - buffered_writer->position;
    if (available_space == 0) {
      buffered_writer_flush(buffered_writer);
      available_space = BUFFERED_WRITER_BUFFER_SIZE;
    }
    int segment_size = size < available_space ? size : available_space;
    memcpy(buffered_writer->buffer + buffered_writer->position, buffer, segment_size);
    buffered_writer->position += segment_size;
    buffer += segment_size;
    size -= segment_size;
  }
}

void buffered_writer_end(buffered_writer_t *buffered_writer) {
  buffered_writer_flush(buffered_writer);
  close_http_connection(buffered_writer->http_server);
}
