//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SHADER_H
#define BINOCLE_SHADER_H

#include "binocle_sdl.h"

typedef struct binocle_shader {
  GLuint vert_id;
  GLuint frag_id;
  GLuint program_id;
  char *vert_src;
  char *frag_src;
} binocle_shader;

typedef enum {
  BINOCLE_SHADER_DEFAULT_FLAT,
  BINOCLE_SHADER_DEFAULT_MAX,
} binocle_shader_defaults_slot;

binocle_shader binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_MAX];

/**
 * \brief Creates a new shader
 * @return a new shader
 */
binocle_shader binocle_shader_new();

/**
 * \brief Loads a shader from filesystem
 * @param vert_filename vertex shader filename
 * @param frag_filename fragment shader filename
 * @return the compiled and linked shader
 */
binocle_shader binocle_shader_load_from_file(char *vert_filename, char *frag_filename);

/**
 * \brief Compiles a shader
 * @param src the source code of the shader
 * @param shader_type the kind of shader, either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * @param shad_id a pointer to the shader id. This value will be written by this function if the compilation is
 * successful
 * @return true if compilation succeeded
 */
bool binocle_shader_compile(const char *src, GLenum shader_type, GLuint *shad_id);

/**
 * Links a shader
 * @param vert_id the vertex shader id
 * @param frag_id the fragment shader id
 * @param prog_id a pointer to the program id that this function will write to if linking is successful
 * @return true if linking succeeded
 */
bool binocle_shader_link(GLuint vert_id, GLuint frag_id, GLuint *prog_id);

/**
 * \brief Frees a shader resources
 * @param shader true if the operation succeeded
 */
void binocle_shader_unload(binocle_shader *shader);

char *str_replace(char *orig, char *rep, char *with);

/**
 * \brief Initializes the default shaders value. This function should be called once before using any of the default
 * shaders
 */
void binocle_shader_init_defaults();

#endif //BINOCLE_SHADER_H
