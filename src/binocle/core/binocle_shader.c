//
// Created by Valerio Santinelli on 19/04/18.
//

#include <stdio.h>
#include <string.h>
#include "binocle_shader.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_gd.h"

binocle_shader binocle_shader_new() {
    binocle_shader res = {0};

    return res;
}

binocle_shader binocle_shader_load_from_file(char *vert_filename, char *frag_filename) {
    char info[1024];
    sprintf(info, "Loading vertex shader %s", vert_filename);
    binocle_log_info(info);
  char opengles_precision[128];
  strcpy(opengles_precision, "precision mediump float;\n\0");
    binocle_shader shader = binocle_shader_new();

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
            shader.vert_src = (char *)SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
            strcpy(shader.vert_src, opengles_precision);
            strcat(shader.vert_src, tmp);
#else
          strcpy(shader.vert_src, tmp);
#endif
        }
    }

  // Load common primitives file to include where needed
  char include_filename[1024];
    char *include_src;
    // TODO: fix this as it's crap as it is now
  sprintf(include_filename, "../Resources/primitives.frag");
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
      include_src = (char *)SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
      strcpy(include_src, opengles_precision);
            strcat(include_src, tmp);
#else
      strcpy(include_src, tmp);
#endif
    }
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
          shader.frag_src = (char *)SDL_malloc(res_size + 1 + sizeof(opengles_precision));
#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
          strcpy(shader.frag_src, opengles_precision);
          strcat(shader.frag_src, tmp);
#else
          strcpy(shader.frag_src, tmp);
#endif
          shader.frag_src = str_replace(shader.frag_src, "#include \"primitives.frag\"", include_src);
        }
    }

    if (shader.vert_src != NULL) {
        sprintf(info, "Compiling vertex shader %s", vert_filename);
        binocle_log_info(info);
        shader.vert_id = binocle_shader_compile(shader.vert_src, GL_VERTEX_SHADER);
    } else {
        binocle_log_warning("Vertex shader not found");
    }

    if (shader.frag_src != NULL) {
        sprintf(info, "Compiling fragment shader %s", frag_filename);
        binocle_log_info(info);
        shader.frag_id = binocle_shader_compile(shader.frag_src, GL_FRAGMENT_SHADER);
    } else {
        binocle_log_warning("Fragment shader not found");
    }

    sprintf(info, "Linking vertex shader %s and fragment shader %s", vert_filename, frag_filename);
    binocle_log_info(info);
    shader.program_id = binocle_shader_link(shader.vert_id, shader.frag_id);

    return shader;
}

GLuint binocle_shader_compile(const char *src, GLenum shader_type) {
    GLuint shader_id = glCreateShader(shader_type);
    glCheck(glShaderSource(shader_id, 1, &src, NULL));
    glCheck(glCompileShader(shader_id));
    GLint status = 0;
    glCheck(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status));
    if (status != GL_TRUE) {
        int len = 0;
        glCheck(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &len));
        char *buf = SDL_malloc(len);
        glCheck(glGetShaderInfoLog(shader_id, len, NULL, buf));
        SDL_Log("%s", buf);
    }

    return shader_id;
}

GLuint binocle_shader_link(GLuint vert_id, GLuint frag_id) {
    GLuint program_id = glCreateProgram();
    glCheck(glAttachShader(program_id, vert_id));
    glCheck(glAttachShader(program_id, frag_id));
    glCheck(glLinkProgram(program_id));
    GLint status = 0;
    glCheck(glGetProgramiv(program_id, GL_LINK_STATUS, &status));
    if (status != GL_TRUE) {
        int len = 0;
        glCheck(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len));
        char *buf = SDL_malloc(len);
        glCheck(glGetProgramInfoLog(program_id, len, NULL, buf));
        SDL_Log("%s", buf);
    }

    return program_id;
}

void binocle_shader_unload(binocle_shader *shader) {
    if (shader->program_id != 0) {
        glCheck(glDeleteProgram(shader->program_id));
        shader->program_id = 0;
    }
    if (shader->vert_id != 0) {
        glCheck(glDeleteShader(shader->vert_id));
        shader->vert_id = 0;
    }
    if (shader->frag_id != 0) {
        glCheck(glDeleteShader(shader->frag_id));
        shader->frag_id = 0;
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
