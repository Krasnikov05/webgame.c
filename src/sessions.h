#ifndef SESSIONS_H
#define SESSIONS_H

#include <time.h>
#include <stdbool.h>

#define USERNAME_MAX_LENGTH 256
#define MAX_PLAYERS_PER_GAME 2
#define MAX_SESSION_COUNT 16
#define MAX_GAME_SESSION_COUNT 8
#define MAX_SESSION_INACTIVE_TIME 300

typedef enum {
  GAME_TYPE_EMPTY,
  GAME_TYPE_TICTACTOE,
} game_type_t;

typedef struct {
  bool is_active;
  bool is_finished;
  void *players[MAX_PLAYERS_PER_GAME];
  bool player_seen_state[MAX_PLAYERS_PER_GAME];
  game_type_t game_type;
  void *game;
} game_session_t;

typedef struct {
  bool is_active;
  int id;
  char username[USERNAME_MAX_LENGTH + 1];
  game_session_t *game_session;
  int player_index;
  time_t last_active;
} session_t;

typedef struct {
  game_session_t *game_sessions;
  session_t *sessions;
} session_manager_t;

char *game_type_to_str(game_type_t game_type);

game_type_t str_to_game_type(char *string);

void init_session_manager(session_manager_t *session_manager);

bool validate_username(char *username);

void update_session(session_t *session);

session_t *new_session(session_manager_t *session_manager, char *username);

session_t *find_session_by_id(session_manager_t *session_manager, int id);

void session_find_game(session_manager_t *session_manager, session_t *session, game_type_t game_type);

void clean_sessions(session_manager_t *session_manager);

void disconnect_from_game_session(session_t *session);

void end_game_session(game_session_t *game_session);

void try_start_game_session(game_session_t *game_session);

#endif
