//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_window.h"
#include "backend/binocle_color.h"
#include "binocle_log.h"
#include "binocle_memory.h"
#include "binocle_sdl.h"
#include <inttypes.h>
#include <stdlib.h>

binocle_window *binocle_window_new(uint32_t width, uint32_t height, char *title) {
  binocle_window *res = SDL_malloc(sizeof(binocle_window));
  SDL_memset(res, 0, sizeof(*res));

  res->width = width;
  res->height = height;
  res->title = SDL_strdup(title);
  res->original_width = width;
  res->original_height = height;
  res->is_fullscreen = false;
  res->current_frame_delta = 0;
  res->framerate_timer = binocle_timer_new();
  res->sample_count = 0;
  res->has_depth_buffer = true;

  // Default to 60 FPS
  binocle_window_set_target_fps(res, 60);

  binocle_window_create(res, title, width, height);
  if (res->window == 0) {
    binocle_log_error("binocle_window_new(): Couldn't create Window");
    binocle_sdl_exit();
  }

  binocle_timer_start(&res->framerate_timer);

  binocle_window_refresh(res);
  return res;
}

void binocle_window_destroy(binocle_window *win) {
#if defined(BINOCLE_GL)
  if (win->gl_context != NULL) {
    SDL_GL_DeleteContext(win->gl_context);
    win->gl_context = NULL;
  }
#endif

  if (win->window != NULL) {
    SDL_DestroyWindow(win->window);
    win->window = NULL;
  }

  if (win->title != NULL) {
    SDL_free(win->title);
    win->title = NULL;
  }

#if defined(BINOCLE_METAL)
  if (win->mtl_view != NULL) {
    SDL_Metal_DestroyView(win->mtl_view);
    win->mtl_view = NULL;
  }
#endif

  SDL_free(win);
}

void binocle_window_refresh(binocle_window *win) {
#if defined(__IPHONEOS__)
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWindowWMInfo(win->window, &info);

  glBindFramebuffer(GL_FRAMEBUFFER, info.info.uikit.framebuffer);
  glBindRenderbuffer(GL_RENDERBUFFER,info.info.uikit.colorbuffer);
#endif
#if defined(BINOCLE_GL)
  SDL_GL_SwapWindow(win->window);
#endif
}

void binocle_window_create(binocle_window *win, char *title, uint32_t width, uint32_t height) {
#if defined(__IPHONEOS__) || defined(__ANDROID__)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__EMSCRIPTEN__)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
#if defined(BINOCLE_GL)
  int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
#elif defined(BINOCLE_METAL)
  int flags = SDL_WINDOW_SHOWN;
#endif

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

#if defined(__ANDROID__) || defined(__IPHONEOS__)
  SDL_SetWindowFullscreen(win->window, SDL_TRUE);
#endif

#if defined(BINOCLE_GL)
  win->gl_context = SDL_GL_CreateContext(win->window);
  if (win->gl_context == 0) {
    binocle_log_error("Window::resize: Couldn't create GL Context");
    binocle_log_error(SDL_GetError());
    return;
  }
#elif defined(BINOCLE_METAL)
  win->mtl_view = SDL_Metal_CreateView(win->window);
#endif


  //Use Vsync (0 = no vsync, 1 = vsync)
#if defined(BINOCLE_GL) && !defined(__EMSCRIPTEN__)
  // Try to enable VSYNC
  if (SDL_GL_SetSwapInterval(1) < 0) {
    binocle_log_warning("Warning: Unable to set VSync!");
    binocle_log_warning(SDL_GetError());
  }
#endif

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

  SDL_version compiled;
  SDL_version linked;
  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);
  binocle_log_info("Compiled SDL version: %d.%d.%d\n",
                   compiled.major, compiled.minor, compiled.patch);
  binocle_log_info("Linked SDL version: %d.%d.%d\n",
                   linked.major, linked.minor, linked.patch);

#if defined(WIN32)
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    binocle_log_info("GLEW error: %s", glewGetErrorString(err));
    binocle_sdl_exit();
  }
  binocle_log_info("Status: Using GLEW %s", glewGetString(GLEW_VERSION));
#endif
#if defined(BINOCLE_GL)
  binocle_log_info("OpenGL version supported by this platform (%s): \n",
                   glGetString(GL_VERSION));
  binocle_log_info("GLSL version supported by this platform (%s): \n",
                   glGetString(GL_SHADING_LANGUAGE_VERSION));
#ifdef GL_NUM_EXTENSIONS
  int n;
  glGetIntegerv(GL_NUM_EXTENSIONS, &n);
  for (int i = 0 ; i < n ; i++) {
    binocle_log_info("%s\n", glGetStringi(GL_EXTENSIONS, i));
  }
#endif
#ifdef GL_NUM_SHADING_LANGUAGE_VERSION
  int n;
  glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSION, n);
  for (int i = 0 ; i < n ; i++) {
    binocle_log_info("%s\n", glGetStringi(GL_SHADING_LANGUAGE_VERSION, i));
  }
#endif
#elif defined(BINOCLE_METAL)
  binocle_log_info("Using Metal backend\n");
#endif
}

void binocle_window_set_background_color(binocle_window *win, sg_color color) {
  win->bg_color = color;
}

void binocle_window_set_title(binocle_window *win, char *title) {
  if (win->window)
    SDL_SetWindowTitle(win->window, title);
}

void binocle_window_toggle_fullscreen(binocle_window *win) {
  SDL_SetWindowFullscreen(win->window, 0);
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
  // Frame time control system
  win->current_time = SDL_GetTicks();
  win->draw_time = win->current_time - win->previous_time;
  win->previous_time = win->current_time;

  win->frame_time = (win->update_time + win->draw_time);

  // Wait for some milliseconds...
  if (win->frame_time < win->target_time) {
    SDL_Delay(win->target_time - win->frame_time);

    win->current_time = SDL_GetTicks();
    double extra_time = win->current_time - win->previous_time;
    win->previous_time = win->current_time;

    win->frame_time += extra_time;
  }

  /*
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
   */
}

uint32_t binocle_window_get_delta(binocle_window *win) {
  return win->current_frame_delta;
}

void binocle_window_set_minimum_size(binocle_window *win, int width, int height) {
  SDL_SetWindowMinimumSize(win->window, width, height);
}

void binocle_window_set_target_fps(binocle_window *win, uint64_t fps) {
  win->target_fps = fps;
  if (fps < 1) {
    win->target_time = 0;
  } else {
    win->target_time = (uint32_t) (1.0 / (double) fps * 1000.0);
  }
  binocle_log_info("Target FPS: %" PRIu64 " and target time per frame: %" PRIu32" milliseconds", win->target_fps,
                   win->target_time);
}

// Returns current FPS
uint64_t binocle_window_get_fps(binocle_window *win) {
  if (binocle_window_get_frame_time(win) > 0) {
    return (uint64_t) (1000.0 / binocle_window_get_frame_time(win));
  } else {
    return 0;
  }
}

// Returns time in milliseconds for last frame drawn
uint32_t binocle_window_get_frame_time(binocle_window *win) {
  // NOTE: We round value to milliseconds
  return win->frame_time;
}

void binocle_window_begin_frame(binocle_window *win) {
  win->current_time = SDL_GetTicks();
  win->update_time = win->current_time - win->previous_time;
  win->previous_time = win->current_time;
}

void binocle_window_end_frame(binocle_window *win) {
  binocle_window_delay_framerate_if_needed(win);
}

void binocle_window_get_real_size(binocle_window *win, uint32_t *w, uint32_t *h) {
  SDL_GetWindowSize(win->window, (int *) w, (int *) h);
}

bool binocle_window_get_display_size(uint32_t *w, uint32_t *h) {
  // NOTE: this only takes into account the first display
  SDL_DisplayMode current;
  for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {

    if (SDL_GetCurrentDisplayMode(i, &current) == 0) {
      *w = current.w;
      *h = current.h;
      return true;
    } else {
      binocle_log_warning("Could not get display mode for video display #%d: %s", i, SDL_GetError());
      return false;
    }
  }
  return false;
}

sg_environment binocle_window_get_environment(binocle_window *window) {
  return (sg_environment) {
    .defaults = {
      .color_format = SG_PIXELFORMAT_RGBA8,
      .depth_format = window->has_depth_buffer ? SG_PIXELFORMAT_NONE : SG_PIXELFORMAT_DEPTH_STENCIL,
      .sample_count = window->sample_count,
  },
};
}
sg_swapchain binocle_window_get_swapchain(binocle_window *window) {
  return (sg_swapchain) {
    .width = window->width,
    .height = window->height,
    .sample_count = window->sample_count,
    .color_format = SG_PIXELFORMAT_RGBA8,
    .depth_format = window->has_depth_buffer ? SG_PIXELFORMAT_NONE : SG_PIXELFORMAT_DEPTH_STENCIL,
    .gl = {
      // we just assume here that the GL framebuffer is always 0
      .framebuffer = 0,
  }
  };
}