//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "binocle_shader.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_gd.h"

const char *binocle_shader_default_vertex_src = "\
\n\
\
attribute vec3 vertexPosition;\
attribute vec2 vertexTCoord;\
attribute vec4 vertexColor;\
attribute vec3 vertexNormal;\
\
varying vec2 tcoord;\
varying vec4 color;\
\
uniform mat4 projectionMatrix;\
uniform mat4 viewMatrix;\
uniform mat4 modelMatrix;\
\
void main(void) {\
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);\
    tcoord = vertexTCoord;\
    color = vertexColor;\
    vec3 n = vertexNormal;\
    gl_PointSize = 1.0;\
}\
\0";

const char *binocle_shader_flat_src = "\
\n\
uniform vec4 color;\
void main(void)\
{\
    gl_FragColor = color;\
}\
\0";


binocle_shader *binocle_shader_new() {
  binocle_shader *res = malloc(sizeof(binocle_shader));
  memset(res, 0, sizeof(*res));
  return res;
}

binocle_shader *binocle_shader_load_from_file(char *vert_filename,
                                             char *frag_filename) {
  char info[1024];
  sprintf(info, "Loading vertex shader %s", vert_filename);
  binocle_log_info(info);
  char opengles_precision[128];
  strcpy(opengles_precision, "precision mediump float;\n\0");
  binocle_shader *shader = binocle_shader_new();

  SDL_RWops *vert_file = SDL_RWFromFile(vert_filename, "rb");
  if (vert_file != NULL) {
    Sint64 res_size = SDL_RWsize(vert_file);
    char *tmp = (char *) SDL_malloc(res_size + 1);

    Sint64 nb_read_total = 0, nb_read = 1;
    char *buf = tmp;
    while (nb_read_total < res_size && nb_read != 0) {
      nb_read = SDL_RWread(vert_file, buf, 1, (res_size - nb_read_total));
      nb_read_total += nb_read;
      buf += nb_read;
    }
    SDL_RWclose(vert_file);
    if (nb_read_total != res_size) {
      SDL_Log("Size mismatch");
      SDL_free(tmp);
    } else {
      tmp[nb_read_total] = '\0';
      shader->vert_src =
          (char *) SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
      strcpy(shader->vert_src, opengles_precision);
      strcat(shader->vert_src, tmp);
#else
      strcpy(shader->vert_src, tmp);
#endif
    }
  }

  // Load common primitives file to include where needed
  char include_filename[1024];
  char *include_src;
  // TODO: fix this as it's crap as it is now
#if defined(WIN32)
  sprintf(include_filename, "../assets/primitives.frag");
#else
  sprintf(include_filename, "../Resources/primitives.frag");
#endif
  SDL_RWops *include_file = SDL_RWFromFile(include_filename, "rb");
  if (include_file != NULL) {
    Sint64 res_size = SDL_RWsize(include_file);
    char *tmp = (char *) SDL_malloc(res_size + 1);

    Sint64 nb_read_total = 0, nb_read = 1;
    char *buf = tmp;
    while (nb_read_total < res_size && nb_read != 0) {
      nb_read = SDL_RWread(include_file, buf, 1, (res_size - nb_read_total));
      nb_read_total += nb_read;
      buf += nb_read;
    }
    SDL_RWclose(include_file);
    if (nb_read_total != res_size) {
      SDL_Log("Size mismatch");
      SDL_free(tmp);
    } else {
      tmp[nb_read_total] = '\0';
      include_src =
          (char *) SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
      strcpy(include_src, opengles_precision);
      strcat(include_src, tmp);
#else
      strcpy(include_src, tmp);
#endif
    }
  } else {
    include_src = (char *) SDL_malloc(1);
    include_src[0] = '\0';
  }

  sprintf(info, "Loading fragment shader %s", frag_filename);
  binocle_log_info(info);
  SDL_RWops *frag_file = SDL_RWFromFile(frag_filename, "rb");
  if (frag_file != NULL) {
    Sint64 res_size = SDL_RWsize(frag_file);
    char *tmp = (char *) SDL_malloc(res_size + 1);

    Sint64 nb_read_total = 0, nb_read = 1;
    char *buf = tmp;
    while (nb_read_total < res_size && nb_read != 0) {
      nb_read = SDL_RWread(frag_file, buf, 1, (res_size - nb_read_total));
      nb_read_total += nb_read;
      buf += nb_read;
    }
    SDL_RWclose(frag_file);
    if (nb_read_total != res_size) {
      SDL_Log("Size mismatch");
      SDL_free(tmp);
    } else {
      tmp[nb_read_total] = '\0';
      shader->frag_src =
          (char *) SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
      strcpy(shader->frag_src, opengles_precision);
      strcat(shader->frag_src, tmp);
#else
      strcpy(shader->frag_src, tmp);
#endif
      shader->frag_src = str_replace(
          shader->frag_src, "#include \"primitives.frag\"", include_src);
    }
  }

  if (shader->vert_src != NULL) {
    sprintf(info, "Compiling vertex shader %s", vert_filename);
    binocle_log_info(info);
    if (!binocle_shader_compile(shader->vert_src, GL_VERTEX_SHADER, &shader->vert_id)) {
      binocle_log_error("Error compiling vertex shader %s", vert_filename);
    }
  } else {
    binocle_log_warning("Vertex shader not found");
  }

  if (shader->frag_src != NULL) {
    sprintf(info, "Compiling fragment shader %s", frag_filename);
    binocle_log_info(info);
    if (!binocle_shader_compile(shader->frag_src, GL_FRAGMENT_SHADER, &shader->frag_id)) {
      binocle_log_error("Error compiling fragment shader %s", frag_filename);
    }
  } else {
    binocle_log_warning("Fragment shader not found");
  }

  sprintf(info, "Linking vertex shader %s and fragment shader %s",
          vert_filename, frag_filename);
  binocle_log_info(info);
  if (!binocle_shader_link(shader->vert_id, shader->frag_id, &shader->program_id)) {
    binocle_log_error("Error linking shader with vertex %s and fragment %s", vert_filename, frag_filename);
  }

  return shader;
}

bool binocle_shader_compile(const char *src, GLenum shader_type, GLuint *shad_id) {
  GLuint shader_id = 0;
  glCheck(shader_id = glCreateShader(shader_type));
  glCheck(glShaderSource(shader_id, 1, &src, NULL));
  glCheck(glCompileShader(shader_id));
  GLint status = 0;
  glCheck(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status));
  if (status != GL_TRUE) {
    GLint len = 0;
    glCheck(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &len));
    if (len > 0) {
      char *buf = SDL_malloc(len);
      glCheck(glGetShaderInfoLog(shader_id, len, NULL, buf));
      binocle_log_error("%s", buf);
      SDL_free(buf);
    } else {
      binocle_log_error("Cannot compile shader but can't retrieve the GL error");
    }
    glCheck(glDeleteShader(shader_id));
    shader_id = 0;
    return false;
  }

  *shad_id = shader_id;
  return true;
}

bool binocle_shader_link(GLuint vert_id, GLuint frag_id, GLuint *prog_id) {
  if (vert_id == 0) {
    binocle_log_error("Cannot link shader as it's got no vertex shader compiled");
    return false;
  }
  if (frag_id == 0) {
    binocle_log_error("Cannot link shader as it's got no fragment shader compiled");
    return false;
  }
  GLuint program_id = 0;
  glCheck(program_id = glCreateProgram());
  glCheck(glAttachShader(program_id, vert_id));
  glCheck(glAttachShader(program_id, frag_id));
  glCheck(glLinkProgram(program_id));
  GLint status = 0;
  glCheck(glGetProgramiv(program_id, GL_LINK_STATUS, &status));
  if (status != GL_TRUE) {
    GLint len = 0;
    glCheck(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len));
    if (len > 0) {
      char *buf = SDL_malloc(len);
      glCheck(glGetProgramInfoLog(program_id, len, NULL, buf));
      binocle_log_error("%s", buf);
      SDL_free(buf);
    } else {
      binocle_log_error("Cannot link shader but can't retrieve the GL error");
    }
    glCheck(glDeleteProgram(program_id));
    program_id = 0;
    glCheck(glDeleteShader(vert_id));
    glCheck(glDeleteShader(frag_id));
    return false;
  }

  glCheck(glDetachShader(program_id, vert_id));
  glCheck(glDetachShader(program_id, frag_id));
  *prog_id = program_id;
  return true;
}

void binocle_shader_unload(binocle_shader *shader) {
  if (shader->program_id != 0) {
    glCheck(glDeleteProgram(shader->program_id));
    shader->program_id = 0;
  } else {
    binocle_log_warning("Trying to delete a program with ID=0");
  }
  if (shader->vert_id != 0) {
    glCheck(glDeleteShader(shader->vert_id));
    shader->vert_id = 0;
  } else {
    binocle_log_warning("Trying to delete a vertex shader with ID=0");
  }
  if (shader->frag_id != 0) {
    glCheck(glDeleteShader(shader->frag_id));
    shader->frag_id = 0;
  } else {
    binocle_log_warning("Trying to delete a fragment shader with ID=0");
  }
}

char *str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep (the string to remove)
  int len_with; // length of with (the string to replace rep with)
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  // sanity checks and initialization
  if (!orig || !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL; // empty rep causes infinite loop during count
  if (!with)
    with = "";
  len_with = strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }
  strcpy(tmp, orig);
  return result;
}

void binocle_shader_init_defaults() {
  binocle_shader *flat = &binocle_shader_defaults[BINOCLE_SHADER_DEFAULT_FLAT];
  char opengles_precision[128];

  flat->vert_src = (char *) SDL_malloc(strlen(binocle_shader_default_vertex_src) + 1 + sizeof(opengles_precision));
  flat->frag_src = (char *) SDL_malloc(strlen(binocle_shader_flat_src) + 1 + sizeof(opengles_precision));

  strcpy(opengles_precision, "precision mediump float;\n\0");

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
  strcpy(flat->vert_src, opengles_precision);
  strcat(flat->vert_src, (char *)binocle_shader_default_vertex_src);

  strcpy(flat->frag_src, opengles_precision);
  strcat(flat->frag_src, (char *)binocle_shader_flat_src);
#else
  strcpy(flat->vert_src, binocle_shader_default_vertex_src);
  strcpy(flat->frag_src, binocle_shader_flat_src);
#endif
  //flat.vert_src = (char *)binocle_shader_default_vertex_src;
  if (!binocle_shader_compile(flat->vert_src, GL_VERTEX_SHADER, &flat->vert_id)) {
    binocle_log_error("Error compiling default flat vertex shader");
  }
  //flat.frag_src = (char *)binocle_shader_flat_src;
  if (!binocle_shader_compile(flat->frag_src, GL_FRAGMENT_SHADER, &flat->frag_id)) {
    binocle_log_error("Error compiling default flat fragment shader");
  }
  if (!binocle_shader_link(flat->vert_id, flat->frag_id, &flat->program_id)) {
    binocle_log_error("Error linking default flat shader");
  }
}

void binocle_shader_destroy(binocle_shader *shader) {
  free(shader);
  shader = NULL;
}