#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sessions.h"
#include "log.h"
#include "tictactoe.h"

char *game_type_to_str(game_type_t game_type) {
  switch (game_type) {
    case GAME_TYPE_EMPTY:
      return NULL;
    case GAME_TYPE_TICTACTOE:
      return "tictactoe";
  }
  return NULL;
}

game_type_t str_to_game_type(char *string) {
  if (strcmp(string, "tictactoe") == 0) {
    return GAME_TYPE_TICTACTOE;
  }
  return GAME_TYPE_EMPTY;
}

void reset_session(session_t *session) {
  disconnect_from_game_session(session);
  session->is_active = false;
}

void init_session_manager(session_manager_t *session_manager) {
  session_manager->game_sessions = NULL;
  session_manager->sessions = NULL;
}

bool validate_username(char *username) {
  if (username == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Expected username got NULL");
    return false;
  }
  if (strlen(username) == 0) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Empty username");
    return false;
  }
  if (strlen(username) > USERNAME_MAX_LENGTH) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Username is too long");
    return false;
  }
  char *c = username;
  while (*c != '\0') {
    switch (*c) {
      case 'A'...'Z':
      case 'a'...'z':
      case '0'...'9':
      case '_':
        c++;
        continue;
      default:
        log_message(LOG_LEVEL_ERROR, "sessions", "Invalid character in username");
        return false;
    }
  }
  return true;
}

void update_session(session_t *session) {
  session->last_active = time(NULL);
}

session_t *new_session(session_manager_t *session_manager, char *username) {
  if (!validate_username(username)) {
    return NULL;
  }
  session_t *session = malloc(sizeof(session_t));
  if (session == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Can't allocate new session");
    exit(1);
  }
  session->next_session = session_manager->sessions;
  session_manager->sessions = session;
  session->is_active = true;
  session->id = rand();
  session->game_session = NULL;
  update_session(session);
  strcpy(session->username, username);
  log_message(LOG_LEVEL_INFO, "sessions", "New session #%d '%s'", session->id, session->username);
  return session; 
}

session_t *find_session_by_id(session_manager_t *session_manager, int id) {
  clean_sessions(session_manager);
  session_t *session = session_manager->sessions;
  while (session != NULL) {
    if (!session->is_active) {
      session = session->next_session;
      continue;
    }
    if (session->id == id) {
      return session;
    }
    session = session->next_session;
  }
  return NULL;
}

void clean_game_sessions(session_manager_t *session_manager) {
  game_session_t **last_ref = &session_manager->game_sessions;
  game_session_t *session = session_manager->game_sessions;
  while (session != NULL) {
    if (!session->removable) {
      *last_ref = session;
      last_ref = (game_session_t **)(&session->next_game_session);
      session = session->next_game_session;
    } else {
      game_session_t *next_session = session->next_game_session;
      free(session);
      session = next_session;
    }
  }
  *last_ref = NULL;
}

void session_find_game(session_manager_t *session_manager, session_t *session, game_type_t game_type) {
  clean_game_sessions(session_manager);
  update_session(session);
  disconnect_from_game_session(session);
  game_session_t *game_session = session_manager->game_sessions;
  while (game_session != NULL) {
    if (
      game_session->is_active ||
      (game_session->game_type != game_type && game_session->game_type != GAME_TYPE_EMPTY)
    ) {
      game_session = game_session->next_game_session;
      continue;
    }
    for (int j = 0; j < MAX_PLAYERS_PER_GAME; j++) {
      if (game_session->players[j] == NULL) {
        session->game_session = game_session;
        session->player_index = j;
        game_session->players[j] = session;
        game_session->game_type = game_type;
        try_start_game_session(game_session);
        return;
      }
    }
    game_session = game_session->next_game_session;
  }
  game_session = malloc(sizeof(game_session_t));
  if (game_session == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Can't allocate new game session");
    exit(1);
  }
  session->game_session = game_session;
  session->player_index = 0;
  game_session->is_active = false;
  game_session->is_finished = false;
  for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
    game_session->player_seen_state[i] = false;
    game_session->players[i] = NULL;
  }
  game_session->players[0] = session;
  game_session->game_type = game_type;
  game_session->game = NULL;
  game_session->removable = false;
  game_session->next_game_session = session_manager->game_sessions;
  session_manager->game_sessions = game_session;
}

void clean_sessions(session_manager_t *session_manager) {
  session_t *session = session_manager->sessions;
  time_t now = time(NULL);
  while (session != NULL) {
    if (!session->is_active) {
      session = session->next_session;
      continue;
    }
    if (now - session->last_active > MAX_SESSION_INACTIVE_TIME) {
      log_message(LOG_LEVEL_INFO, "sessions", "Cleaning session #%d", session->id);
      reset_session(session);
    }
    session = session->next_session;
  }
  session_t **last_ref = &session_manager->sessions;
  session = session_manager->sessions;
  while (session != NULL) {
    if (session->is_active) {
      *last_ref = session;
      last_ref = (session_t **)(&session->next_session);
      session = session->next_session;
    } else {
      session_t *next_session = session->next_session;
      free(session);
      session = next_session;
    }
  }
  *last_ref = NULL;
}

void disconnect_from_game_session(session_t *session) {
  if (session->game_session == NULL) {
    return;
  }
  session->game_session->players[session->player_index] = NULL;
  switch (session->game_session->game_type) {
    case GAME_TYPE_EMPTY:
      break;
    case GAME_TYPE_TICTACTOE:
      tictactoe_handle_disconnect(session->game_session, session);
      break;
  }
  end_game_session(session->game_session);
  session->game_session = NULL;
}

void end_game_session(game_session_t *game_session) {
  if (!game_session->is_active) {
    return;
  }
  if (!game_session->is_finished) {
    for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
      game_session->player_seen_state[i] = false;
    }
  }
  game_session->is_finished = true;
  for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
    if (game_session->players[i] == NULL) {
      continue;
    }
    if (!game_session->player_seen_state[i]) {
      return;
    }
  }
  for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
    session_t *session = game_session->players[i];
    if (session == NULL) {
      continue;
    }
    session->game_session = NULL;
  }
  if (game_session->game != NULL) {
    switch (game_session->game_type) {
      case GAME_TYPE_EMPTY:
        break;
      case GAME_TYPE_TICTACTOE:
        tictactoe_deinit(game_session);
        break;
    }
  }
  game_session->removable = true;
}

void try_start_game_session(game_session_t *game_session) {
  for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
    if (game_session->players[i] == NULL) {
      return;
    }
  }
  game_session->is_active = true;
  switch (game_session->game_type) {
    case GAME_TYPE_EMPTY:
      log_message(LOG_LEVEL_ERROR, "sessions", "Attempting to start GAME_TYPE_EMPTY");
      break;
    case GAME_TYPE_TICTACTOE:
      tictactoe_init(game_session);
      break;
  }
}
