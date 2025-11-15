#include <stdlib.h>
#include "arena.h"
#include "log.h"

void init_arena(arena_t *arena, int size) {
  arena->size = size;
  arena->position = 0;
  arena->buffer = (char *)malloc(size);
  if (arena->buffer == NULL) {
    log_message(LOG_LEVEL_ERROR, "arena", "Failed allocating %d bytes for arena", size);
    exit(EXIT_FAILURE);
  }
  log_message(LOG_LEVEL_INFO, "arena", "Allocated %d bytes for arena", size);
}
void deinit_arena(arena_t *arena) {
  free(arena->buffer);
}

char *alloc_arena(arena_t *arena, int size) {
  if (arena->position + size > arena->size) {
    log_message(
      LOG_LEVEL_ERROR,
      "arena",
      "Failed allocating %d bytes on %d bytes arena",
      size,
      arena->size
    );
    return NULL;
  }
  char *res = arena->buffer + arena->position;
  arena->position += size + (8 - size % 8);
  return res;
}

void free_arena(arena_t *arena) {
  arena->position = 0;
}
