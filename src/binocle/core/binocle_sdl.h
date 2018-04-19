//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_BINOCLE_SDL_H
#define BINOCLE_BINOCLE_SDL_H

#include <stdbool.h>
#include <sys/stat.h>

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)

#include <SDL_opengles.h>
#include <SDL_opengles2.h>

#else

//#include <SDL_opengl.h>
#include <OpenGL/gl.h>

#endif

#if defined(__WINDOWS__)
#include <sdl/include/SDL_syswm.h>
#endif

// SDL 2.0
#include <SDL.h>            // SDL2
#include <SDL_mixer.h>      // SDL2_Mixer

/// Starts SDL2, MUST be called before everything else.
bool binocle_sdl_init();

/// Destroys SDL2, MUST be called when program ends.
void binocle_sdl_exit();

time_t binocle_sdl_get_last_write_time(char *Filename);

#endif //BINOCLE_BINOCLE_SDL_H
