//
// Created by Valerio Santinelli on 13/04/18.
//

#include "binocle_log.h"
#include <SDL.h>

void binocle_log_error(const char *msg) {
  SDL_Log("%s", msg);
}

void binocle_log_warning(const char *msg) {
  SDL_Log("%s", msg);
}

void binocle_log_info(const char *msg) {
  SDL_Log("%s", msg);
}

void binocle_log_debug(const char *msg) {
  SDL_Log("%s", msg);
}
