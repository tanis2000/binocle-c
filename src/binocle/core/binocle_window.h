//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_BINOCLE_WINDOW_H
#define BINOCLE_BINOCLE_WINDOW_H

#include <stdbool.h>

#include "binocle_sdl.h"
#include "binocle_timer.h"
#include "binocle_color.h"

typedef struct binocle_window {
  uint32_t width;
  uint32_t height;
  char* title;
  uint32_t original_width;
  uint32_t original_height;
  bool is_fullscreen;
  binocle_color bg_color;
  SDL_GLContext gl_context;
  uint32_t framerate;
  uint32_t frame_delay;
  binocle_timer framerate_timer;
  uint32_t current_frame_delta;
  SDL_Window *window;
} binocle_window;

binocle_window binocle_window_new(uint32_t width, uint32_t height, char* title);
void binocle_window_resize(binocle_window *win, char *title, uint32_t width, uint32_t height);
void binocle_window_set_background_color(binocle_window* win, binocle_color color);
void binocle_window_clear(binocle_window *win);
void binocle_window_refresh(binocle_window *win);
void binocle_window_set_title(binocle_window *win, char* title);
void binocle_window_toggle_fullscreen(binocle_window *win);
void binocle_window_minimize(binocle_window *win);
void binocle_window_maximize(binocle_window *win);
void binocle_window_restore(binocle_window *win);
void binocle_window_delay_framerate_if_needed(binocle_window *win);
uint32_t binocle_window_get_delta(binocle_window *win);
void binocle_window_set_minimum_size(binocle_window *win, int width, int height);

#endif //BINOCLE_BINOCLE_WINDOW_H
