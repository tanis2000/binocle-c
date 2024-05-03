//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_log.h"
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define BINOCLE_LOG_BUFFER_SIZE 65535

/// the temporary buffer used to print the log messages
static char *log_buffer = NULL;

void binocle_log_error(const char *msg, ...) {
  binocle_log_ensure_buffer_is_set();
  va_list args;
  va_start(args, msg);
  vsnprintf(log_buffer, BINOCLE_LOG_BUFFER_SIZE, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_ERROR, "%s", log_buffer);
#else
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
#endif
  va_end(args);
}

void binocle_log_warning(const char *msg, ...) {
  binocle_log_ensure_buffer_is_set();
  va_list args;
  va_start(args, msg);
  vsnprintf(log_buffer, BINOCLE_LOG_BUFFER_SIZE, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_WARN, "%s", log_buffer);
#else
  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
#endif
  va_end(args);
}

void binocle_log_info(const char *msg, ...) {
  binocle_log_ensure_buffer_is_set();
  va_list args;
  va_start(args, msg);
  vsnprintf(log_buffer, BINOCLE_LOG_BUFFER_SIZE, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_CONSOLE, "%s", log_buffer);
#else
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
#endif
  va_end(args);
}

void binocle_log_debug(const char *msg, ...) {
  binocle_log_ensure_buffer_is_set();
  va_list args;
  va_start(args, msg);
  vsnprintf(log_buffer, BINOCLE_LOG_BUFFER_SIZE, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_CONSOLE, "%s", log_buffer);
#else
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
#endif
  va_end(args);
}

void binocle_log_ensure_buffer_is_set() {
  if (log_buffer == NULL) {
    log_buffer = SDL_malloc(BINOCLE_LOG_BUFFER_SIZE);
  }
}