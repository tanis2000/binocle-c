//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SDL_H
#define BINOCLE_SDL_H

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

/**
 * \brief Starts SDL2 and must be called before everything else
 * @return true if everything went ok
 */
bool binocle_sdl_init();

/**
 * \brief Shuts down SDL2 and must be called when the program exits
 */
void binocle_sdl_exit();

/**
 * \brief Gets the last modification time of a given file
 * @param filename The filename
 * @return The last modification time of the file
 */
time_t binocle_sdl_get_last_modification_time(char *filename);

/**
 * \brief Loads a text file
 * Loads a text file and stores it in buffer. This function allocates memory so it's up to the consumer to free
 * it when done with the content.
 * @param filename The filename
 * @param buffer The buffer to write the output to
 * @param buffer_length The size of the buffer
 * @return true if everything went ok
 */
bool binocle_sdl_load_text_file(char *filename, char **buffer, size_t *buffer_length);

/**
 * Writes a text to a file
 * @param filename The filename
 * @param buffer The buffer with the text
 * @param size The size of the buffer
 * @return true if everything went ok
 */
bool binocle_sdl_write_text_file(char *filename, char *buffer, size_t size);

#endif //BINOCLE_SDL_H
