//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_BINOCLE_SHADER_H
#define BINOCLE_BINOCLE_SHADER_H

#include "binocle_sdl.h"

typedef struct binocle_shader {
    GLuint vert_id;
    GLuint frag_id;
    GLuint program_id;
    char *vert_src;
    char *frag_src;
} binocle_shader;

binocle_shader binocle_shader_new();
binocle_shader binocle_shader_load_from_file(char *vert_filename, char *frag_filename);
GLuint binocle_shader_compile(const char *src, GLenum shader_type);
GLuint binocle_shader_link(GLuint vert_id, GLuint frag_id);
void binocle_shader_unload(binocle_shader *shader);

#endif //BINOCLE_BINOCLE_SHADER_H
