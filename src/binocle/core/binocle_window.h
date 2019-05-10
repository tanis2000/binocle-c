//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_WINDOW_H
#define BINOCLE_WINDOW_H

#include <stdbool.h>

#include "binocle_sdl.h"
#include "binocle_timer.h"
#include "binocle_color.h"

typedef struct binocle_window {
  uint32_t width;
  uint32_t height;
  char *title;
  uint32_t original_width;
  uint32_t original_height;
  bool is_fullscreen;
  binocle_color bg_color;
  SDL_GLContext gl_context;

  uint64_t target_fps; // The desired target FPS
  uint32_t current_time;            // Current time measure in ticks (ms)
  uint32_t previous_time;           // Previous time measure in ticks (ms)
  uint32_t update_time;             // Time measure for frame update in ticks (ms)
  uint32_t draw_time;               // Time measure for frame draw in ticks (ms)
  uint32_t frame_time;              // Time measure for one frame in ticks (ms)
  uint32_t target_time;             // Desired time for one frame in ticks (ms), if 0 not applied

  binocle_timer framerate_timer;
  uint32_t current_frame_delta;


  SDL_Window *window;
} binocle_window;

binocle_window binocle_window_new(uint32_t width, uint32_t height, char *title);

void binocle_window_resize(binocle_window *win, char *title, uint32_t width, uint32_t height);

void binocle_window_set_background_color(binocle_window *win, binocle_color color);

void binocle_window_clear(binocle_window *win);

void binocle_window_refresh(binocle_window *win);

void binocle_window_set_title(binocle_window *win, char *title);

void binocle_window_toggle_fullscreen(binocle_window *win);

void binocle_window_minimize(binocle_window *win);

void binocle_window_maximize(binocle_window *win);

void binocle_window_restore(binocle_window *win);

void binocle_window_delay_framerate_if_needed(binocle_window *win);

uint32_t binocle_window_get_delta(binocle_window *win);

void binocle_window_set_minimum_size(binocle_window *win, int width, int height);

void binocle_window_set_target_fps(binocle_window *win, uint64_t fps);

uint64_t binocle_window_get_fps(binocle_window *win);

uint32_t binocle_window_get_frame_time(binocle_window *win);

void binocle_window_begin_frame(binocle_window *win);

void binocle_window_end_frame(binocle_window *win);

void binocle_window_get_real_size(binocle_window *win, uint32_t *w, uint32_t *h);

bool binocle_window_get_display_size(uint32_t *w, uint32_t *h);

#endif //BINOCLE_WINDOW_H
