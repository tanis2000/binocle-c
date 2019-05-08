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
#include <SDL_syswm.h>

#elif defined(__APPLE__)

//#include <SDL_opengl.h>
#include <OpenGL/gl.h>

#endif

#if defined(__WINDOWS__)
#include <glew/include/GL/glew.h>
#include <SDL_opengl.h>
#include <sdl/include/SDL_syswm.h>
#endif

// SDL 2.0
#include <SDL.h>            // SDL2
//#include <SDL_mixer.h>      // SDL2_Mixer

/// Starts SDL2, MUST be called before everything else.
bool binocle_sdl_init();

/// Destroys SDL2, MUST be called when program ends.
void binocle_sdl_exit();

time_t binocle_sdl_get_last_write_time(char *Filename);

bool binocle_sdl_load_text_file(char *filename, char **buffer, size_t *buffer_length);

bool binocle_sdl_write_text_file(char *filename, char *buffer, size_t size);

#endif //BINOCLE_BINOCLE_SDL_H
