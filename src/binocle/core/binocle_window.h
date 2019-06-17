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

/**
 * This is the representation of the app/window. Currently an app can only have one window
 */
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

/**
 * \brief Creates a new window
 * @param width the width of the window
 * @param height the height of the window
 * @param title the title of the window
 * @return a window instance
 */
binocle_window binocle_window_new(uint32_t width, uint32_t height, char *title);

/**
 * \brief Resizes a window and resets the title
 * @param win the original window
 * @param title the new title
 * @param width the new width
 * @param height the new height
 */
void binocle_window_resize(binocle_window *win, char *title, uint32_t width, uint32_t height);

/**
 * \brief Sets the background color of the window
 * @param win the window
 * @param color the color
 */
void binocle_window_set_background_color(binocle_window *win, binocle_color color);

/**
 * \brief Clears the window content (direct OpenGL calls)
 * @param win the window
 */
void binocle_window_clear(binocle_window *win);

/**
 * \brief Switches the buffers as needed to show the new frame in the given window
 * @param win the window
 */
void binocle_window_refresh(binocle_window *win);

/**
 * \brief Sets the title of a window
 * @param win the window
 * @param title the title
 */
void binocle_window_set_title(binocle_window *win, char *title);

/**
 * \brief Toggles fullscreen
 * @param win the window
 */
void binocle_window_toggle_fullscreen(binocle_window *win);

/**
 * \brief Minimizes the window
 * @param win the window
 */
void binocle_window_minimize(binocle_window *win);

/**
 * \brief Maximizes the window
 * @param win the window
 */
void binocle_window_maximize(binocle_window *win);

/**
 * \brief Restores the size and position of a minimized or maximized window
 * @param win the window
 */
void binocle_window_restore(binocle_window *win);

/**
 * \brief Delays the current thread to meet the frame rate requirements, if needed
 * @param win the window
 */
void binocle_window_delay_framerate_if_needed(binocle_window *win);

/**
 * \brief Gets the delta time since the last frame in ticks (ms)
 * @param win the window
 * @return the delta time in ticks (ms)
 */
uint32_t binocle_window_get_delta(binocle_window *win);

/**
 * \brief Sets the minimum size of the window
 * @param win the window
 * @param width the minimum width
 * @param height the minimum height
 */
void binocle_window_set_minimum_size(binocle_window *win, int width, int height);

/**
 * \brief Sets the target FPS for the application
 * When this is called, the main loop will try to meet the desired FPS and delay the main thread if needed
 * @param win the window
 * @param fps the FPS
 */
void binocle_window_set_target_fps(binocle_window *win, uint64_t fps);

/**
 * \brief Gets the current FPS
 * @param win the window
 * @return the FPS
 */
uint64_t binocle_window_get_fps(binocle_window *win);

/**
 * \brief Gets how long the last frame took, in ticks (ms)
 * @param win the window
 * @return how long the last frame took, in ticks (ms)
 */
uint32_t binocle_window_get_frame_time(binocle_window *win);

/**
 * \brief Instructs the application that we are about to start a new frame.
 * This is needed if we want to keep a steady frame rate and should be called at the beginning of your game loop
 * @param win the window
 */
void binocle_window_begin_frame(binocle_window *win);

/**
 * \brief Instructs the application that we have just finished with the current frame.
 * This is needed if we want to keep a steady frame rate and should be called at the end of your game loop
 * @param win the window
 */
void binocle_window_end_frame(binocle_window *win);

/**
 * \brief Gets the size of the window client area
 * @param win the window
 * @param w a pointer to the width where this value will be written to
 * @param h a pointer to the height where this value will be written to
 */
void binocle_window_get_real_size(binocle_window *win, uint32_t *w, uint32_t *h);

/**
 * \brief Gets the size of the display this window is being displayed on.
 * Warning: this currently takes into account only the first display.
 * @param w a pointer to the width where this value will be written to
 * @param h a pointer to the height where this value will be written to
 * @return true if the operation finished successfully
 */
bool binocle_window_get_display_size(uint32_t *w, uint32_t *h);

#endif //BINOCLE_WINDOW_H
