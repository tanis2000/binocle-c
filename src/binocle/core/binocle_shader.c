//
// Created by Valerio Santinelli on 19/04/18.
//

#include "binocle_shader.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_gd.h"

binocle_shader binocle_shader_new() {
    binocle_shader res = {
    };

    return res;
}

binocle_shader binocle_shader_load_from_file(char *vert_filename, char *frag_filename) {
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
        }
    }

    if (shader.vert_src != NULL) {
        shader.vert_id = binocle_shader_compile(shader.vert_src, GL_VERTEX_SHADER);
    } else {
        binocle_log_warning("Vertex shader not loaded");
    }

    if (shader.frag_src != NULL) {
        shader.frag_id = binocle_shader_compile(shader.frag_src, GL_FRAGMENT_SHADER);
    } else {
        binocle_log_warning("Fragment shader not loaded");
    }

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