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

void binocle_log_error(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_ERROR, "%s", formatted_string);
#else
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
#endif
  va_end(args);
}

void binocle_log_warning(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_WARN, "%s", formatted_string);
#else
  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
#endif
  va_end(args);
}

void binocle_log_info(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_CONSOLE, "%s", formatted_string);
#else
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
#endif
  va_end(args);
}

void binocle_log_debug(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
#ifdef __EMSCRIPTEN__
  emscripten_log(EM_LOG_CONSOLE, "%s", formatted_string);
#else
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
#endif
  va_end(args);
}
