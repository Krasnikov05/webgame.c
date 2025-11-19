#ifndef TICTACTOE_H
#define TICTACTOE_H

#include "sessions.h"
#include "json_writer.h"

typedef enum {
  TICTACTOE_CELL_EMPTY,
  TICTACTOE_CELL_X,
  TICTACTOE_CELL_O,
} tictactoe_cell_t;

typedef struct {
  int active_player_index;
  tictactoe_cell_t cells[9];
} tictactoe_game_t;

void tictactoe_init(game_session_t *game_session);

void tictactoe_deinit(game_session_t *game_session);

void tictactoe_write_json(game_session_t *game_session, json_writer_t *json_writer);

void tictactoe_handle_request(game_session_t *game_session, session_t *session, http_server_t *http_server);

#endif
