//
// Created by Valerio Santinelli on 13/04/18.
//

#include "binocle_sdl.h"
#include "binocle_color.h"
#include "binocle_window.h"
#include "binocle_log.h"

binocle_window binocle_window_new(uint32_t width, uint32_t height, char* title) {
  binocle_window res = {};
  res.width = width;
  res.height = height;
  res.title = title;
  res.original_width = width;
  res.original_height = height;
  res.is_fullscreen = false;
  res.framerate = 60;
  res.frame_delay = 0;
  res.current_frame_delta = 0;
  res.framerate_timer = binocle_timer_new();

  binocle_window_resize(&res, title, width, height);
  if (res.window == 0) {
    binocle_log_error("Window(): Couldn't create Window");
    binocle_sdl_exit();
  }

  binocle_timer_start(&res.framerate_timer);

  binocle_window_clear(&res);
  binocle_window_refresh(&res);
  return res;
}

void binocle_window_fill(binocle_color color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

void binocle_window_clear(binocle_window *win) {
  binocle_window_fill(win->bg_color);
}

void binocle_window_refresh(binocle_window *win) {
  SDL_GL_SwapWindow(win->window);
}

void binocle_window_destroy(binocle_window *win) {
  if (win->gl_context) {
    SDL_GL_DeleteContext(win->gl_context);
    win->gl_context = 0;
  }

  if (win->window) {
    SDL_DestroyWindow(win->window);
    win->window = 0;
  }
}


void binocle_window_resize(binocle_window *win, char* title, uint32_t width, uint32_t height) {
  // Just in case we already have a window
  binocle_window_destroy(win);
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
#endif

  int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
#if defined(__IPHONEOS__) || defined(__ANDROID__)
  flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN;
#else
  flags |= SDL_WINDOW_RESIZABLE;
#endif

  SDL_DisplayMode mode;
  SDL_GetDesktopDisplayMode(0, &mode);

  /*
#ifdef ANDROID
  screen.w = mode.w;
  screen.h = mode.h;
#else
  screen.w = (int)(mode.h - 200) / 1.5;
  screen.h = mode.h - 200;
#endif
*/
  win->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                 flags);

  if (win->window == 0) {
    binocle_log_error("Window::resize: Couldn't create SDL_Window");
    return;
  }

#if defined(__ANDROID__)
  SDL_SetWindowFullscreen(window, SDL_TRUE);
#endif

  win->gl_context = SDL_GL_CreateContext(win->window);
  if (win->gl_context == 0) {
    binocle_log_error("Window::resize: Couldn't create GL Context");
    binocle_log_error(SDL_GetError());
    return;
  }

  //Use Vsync
  if (SDL_GL_SetSwapInterval(1) < 0) {
    binocle_log_warning("Warning: Unable to set VSync!");
    binocle_log_warning(SDL_GetError());
  }

  // And here we fake a "logical" size of the window,
  // independent of it's real size.
  int w = 0;
  int h = 0;
  SDL_GetWindowSize(win->window, &w, &h);
  //SDL_RenderSetLogicalSize(this->renderer, width, height);

  // Lil' title
  binocle_window_set_title(win, title);

  /*this->surface  = SDL_GetWindowSurface(this->window);
      if (!(this->surface)) {
      Log::error("Window::resize: couldn't get surface");
      Log::error(SDL_GetError());
      return;
      }*/

  /*
  SDL_GL_GetDrawableSize(this->window, &w, &h);
  width = w;
  height = h;
      */

  win->width = width;
  win->height = height;


}

void binocle_window_set_background_color(binocle_window* win, binocle_color color) {
  win->bg_color = color;
}

void binocle_window_set_title(binocle_window *win, char* title) {
  if (win->window)
    SDL_SetWindowTitle(win->window, title);
}

void binocle_window_toggle_fullscreen(binocle_window *win) {
  SDL_SetWindowFullscreen(win, 0);
}

void binocle_window_minimize(binocle_window *win) {
  SDL_MinimizeWindow(win->window);
}

void binocle_window_maximize(binocle_window *win) {
  SDL_MaximizeWindow(win->window);
}

void binocle_window_restore(binocle_window *win) {
  SDL_RestoreWindow(win->window);
}

void binocle_window_delay_framerate_if_needed(binocle_window *win) {
  // How many milliseconds the last frame took
  win->current_frame_delta = binocle_timer_delta(&win->framerate_timer);

  win->frame_delay = 1000.0f / win->framerate;
  if (win->frame_delay < win->current_frame_delta) {
    win->frame_delay = 0;
  } else {
    win->frame_delay = win->frame_delay - win->current_frame_delta;
  }


  if ((win->current_frame_delta) < (win->frame_delay)) {
    //Log::debug(std::to_string(this->frame_delay));
    SDL_Delay((win->frame_delay) - win->current_frame_delta);
  }

  binocle_timer_restart(&win->framerate_timer);
}

uint32_t binocle_window_get_delta(binocle_window *win) {
  return win->current_frame_delta;
}

void binocle_window_set_minimum_size(binocle_window *win, int width, int height) {
  SDL_SetWindowMinimumSize(win->window, width, height);
}
