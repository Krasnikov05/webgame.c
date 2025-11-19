#include <stdio.h>
#include <stdlib.h>
#include "tictactoe.h"
#include "log.h"
#include "http.h"

void tictactoe_init(game_session_t *game_session) {
  log_message(LOG_LEVEL_INFO, "tictactoe", "Creating new game");
  tictactoe_game_t *game = malloc(sizeof(tictactoe_game_t)); 
  if (game == NULL) {
    log_message(LOG_LEVEL_ERROR, "tictactoe", "Failed allocating tictactoe_game_t");
    return;
  }
  game->active_player_index = 0;
  game->winner = -1;
  for (int i = 0; i < 9; i++) {
    game->cells[i] = TICTACTOE_CELL_EMPTY;
  }
  game_session->game = game;
}

void tictactoe_deinit(game_session_t *game_session) {
  log_message(LOG_LEVEL_INFO, "tictactoe", "Deinitializing game");
  if (game_session->game == NULL) {
    return;
  }
  free(game_session->game);
}

void tictactoe_write_json(game_session_t *game_session, json_writer_t *json_writer) {
  tictactoe_game_t *game = game_session->game;
  json_start_dict(json_writer);
  json_write_key(json_writer, "active_player_index");
  json_write_number(json_writer, game->active_player_index);
  json_write_key(json_writer, "cells");
  json_start_array(json_writer);
  for (int i = 0; i < 9; i++) {
    char *value;
    switch (game->cells[i]) {
      case TICTACTOE_CELL_EMPTY:
        value = " ";
        break;
      case TICTACTOE_CELL_X:
        value = "X";
        break;
      case TICTACTOE_CELL_O:
        value = "O";
        break;
    }
    json_write_string(json_writer, value);
  }
  json_stop_array(json_writer);
  json_stop_dict(json_writer);
}

tictactoe_cell_t tictactoe_check_line(tictactoe_game_t *game, int start, int step) {
  tictactoe_cell_t first_cell = game->cells[start];
  if (first_cell == TICTACTOE_CELL_EMPTY) {
    return TICTACTOE_CELL_EMPTY;
  }
  for (int i = 1; i < 3; i++) {
    tictactoe_cell_t cell = game->cells[start + i * step];
    if (cell != first_cell) {
      return TICTACTOE_CELL_EMPTY;
    }
  }
  return first_cell;
}

tictactoe_cell_t tictactoe_check_all_lines(tictactoe_game_t *game) {
  tictactoe_cell_t result = tictactoe_check_line(game, 0, 4);
  if (result != TICTACTOE_CELL_EMPTY) {
    return result;
  }
  result = tictactoe_check_line(game, 2, 2);
  if (result != TICTACTOE_CELL_EMPTY) {
    return result;
  }
  for (int i = 0; i < 3; i++) {
    result = tictactoe_check_line(game, i * 3, 1);
    if (result != TICTACTOE_CELL_EMPTY) {
      return result;
    }
    result = tictactoe_check_line(game, i, 3);
    if (result != TICTACTOE_CELL_EMPTY) {
      return result;
    }
  }
  return TICTACTOE_CELL_EMPTY;
}

void tictactoe_try_finish(game_session_t *game_session) {
  tictactoe_game_t *game = game_session->game;
  tictactoe_cell_t winner_cell = tictactoe_check_all_lines(game);
  if (winner_cell == TICTACTOE_CELL_X) {
    game->winner = 0;
  } else if (winner_cell == TICTACTOE_CELL_O) {
    game->winner = 1;
  }
  if (winner_cell != TICTACTOE_CELL_EMPTY) {
    end_game_session(game_session);
    return;
  }
  for (int i = 0; i < 9; i++) {
    if (game->cells[i] == TICTACTOE_CELL_EMPTY) {
      return;
    }
  }
  end_game_session(game_session);
}

void tictactoe_handle_request(game_session_t *game_session, session_t *session, http_server_t *http_server) {
  char *cell_str = get_http_parameter(http_server, "cell");
  if (cell_str == NULL) {
    return;
  }
  int cell = atoi(cell_str);
  if (cell < 0 || cell >= 9) {
    log_message(LOG_LEVEL_INFO, "tictactoe", "Invalid move cell=%d", cell);
  }
  tictactoe_game_t *game = game_session->game;
  if (game->active_player_index != session->player_index) {
    return;
  }
  if (game->cells[cell] != TICTACTOE_CELL_EMPTY) {
    return;
  }
  game->cells[cell] = session->player_index == 1 ? TICTACTOE_CELL_O : TICTACTOE_CELL_X;
  game->active_player_index = (game->active_player_index + 1) % 2;
  tictactoe_try_finish(game_session);
}
