#ifndef ARENA_H
#define ARENA_H

typedef struct {
  int size;
  int position;
  char *buffer;
} arena_t;

void init_arena(arena_t *arena, int size);

void deinit_arena(arena_t *arena);

char *alloc_arena(arena_t *arena, int size);

void free_arena(arena_t *arena);

#endif
