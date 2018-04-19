//
// Created by Valerio Santinelli on 13/04/18.
//

#include "binocle_log.h"
#include <SDL.h>
#include <stdio.h>
#include <stdarg.h>

void binocle_log_error(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
  va_end(args);
}

void binocle_log_warning(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
  va_end(args);
}

void binocle_log_info(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
  va_end(args);
}

void binocle_log_debug(const char *msg, ...) {
  char formatted_string[1024];
  va_list args;
  va_start(args, msg);
  vsnprintf(formatted_string, 1024, msg, args);
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", formatted_string);
  va_end(args);
}
