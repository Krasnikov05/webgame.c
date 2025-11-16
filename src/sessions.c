#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sessions.h"
#include "log.h"

void reset_game_session(game_session_t *game_session) {
  game_session->is_active = false;
  game_session->game_type = GAME_TYPE_EMPTY;
  for (int i = 0; i < MAX_PLAYERS_PER_GAME; i++) {
    game_session->players[i] = NULL;
  }
  game_session->game = NULL;
}

void reset_session(session_t *session) {
  session->is_active = false;
  if (session->game_session == NULL || !session->game_session->is_active) {
    return;
  }
}

void init_session_manager(session_manager_t *session_manager) {
  session_manager->game_sessions = calloc(MAX_GAME_SESSION_COUNT, sizeof(game_session_t));
  if (session_manager->game_sessions == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Failed allocating %d game_sessions", MAX_GAME_SESSION_COUNT);
    exit(EXIT_FAILURE);
  }
  session_manager->sessions = calloc(MAX_SESSION_COUNT, sizeof(session_t));
  if (session_manager->sessions == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Failed allocating %d sessions", MAX_SESSION_COUNT);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < MAX_GAME_SESSION_COUNT; i++) {
    reset_game_session(&session_manager->game_sessions[i]);
  }
  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
    reset_session(&session_manager->sessions[i]);
  }
}

bool validate_username(char *username) {
  if (username == NULL) {
    log_message(LOG_LEVEL_ERROR, "sessions", "Expected username got NULL");
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
  session_t *session;
  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
    if (!session_manager->sessions[i].is_active) {
      session = &session_manager->sessions[i];
      session->is_active = true;
      session->id = rand();
      session->game_session = NULL;
      update_session(session);
      strcpy(session->username, username);
      log_message(LOG_LEVEL_INFO, "sessions", "New session #%d '%s'", session->id, session->username);
      return session; 
    }
  }
  log_message(LOG_LEVEL_ERROR, "sessions", "Session_manager.session is full");
  return NULL;
}

void session_find_game(session_manager_t *session_manager, int session_id, game_type_t game_type);

void clean_sessions(session_manager_t *session_manager) {
  time_t now = time(NULL);
  for (int i = 0; i < MAX_SESSION_COUNT; i++) {
    if (now - session_manager->sessions[i].last_active > MAX_SESSION_INACTIVE_TIME) {
      reset_session(&session_manager->sessions[i]);
    }
  }  
}
