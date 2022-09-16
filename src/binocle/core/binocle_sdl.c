//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_audio.h"
#include <sys/types.h>
#include <sys/stat.h>

bool binocle_sdl_init() {

  // Will initialize subsystems separatedly

  // Make sure we don't end up using Retina
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

  // Make sure textures are being created using nearest pixel sampling
  // 0 = nearest pixel sampling
  // 1 = linear filtering
  // 2 = anisotropic filtering
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

#if defined(__IPHONEOS__) || defined(__ANDROID__)
  /*
    LandscapeLeft - top of device left
    LandscapeRight - top of device right
    Portrait - top of device up
    PortraitUpsideDown - top of device down
    The list is separated by spaces
    */
  //SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
#endif

#if defined(BINOCLE_GL)
#if defined(__IPHONEOS__) || defined(__ANDROID__)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#elif defined(__EMSCRIPTEN__)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles3");
#else
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
#elif defined(BINOCLE_METAL)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
#endif

#ifdef DEBUG
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

  uint32_t to_check = (SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (SDL_Init(to_check) < 0) {
    binocle_log_error("SDL_Init: Couldn't start SDL");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }

  uint32_t check_result = SDL_WasInit(to_check);
  if (!(check_result & SDL_INIT_VIDEO)) {
    binocle_log_error("SDL_WasInit: Video not initialized");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }
  if (!(check_result & SDL_INIT_EVENTS)) {
    binocle_log_error("SDL_WasInit: Events not initialized");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }

  char *base_path = SDL_GetBasePath();
  if (base_path) {
    binocle_log_info("Base path: %s", base_path);
    SDL_free(base_path);
  } else {
    binocle_log_info("Base path not available");
  }


  return true;
}

void binocle_sdl_exit() {
  SDL_Quit();
}

time_t binocle_sdl_get_last_modification_time(char *filename) {
  time_t lastWriteTime = 0;

  struct stat fileStatus;
  if (stat(filename, &fileStatus) == 0) {
    lastWriteTime = fileStatus.st_mtime;
  }

  return (lastWriteTime);
}

bool binocle_sdl_load_text_file(char *filename, char **buffer, size_t *buffer_length) {
  binocle_log_info("Loading text file: %s", filename);
  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open text file");
    return false;
  }

  Sint64 res_size = SDL_RWsize(file);
  char *res = (char *) SDL_malloc(res_size + 1);

  Sint64 nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = SDL_RWread(file, buf, 1, (res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }
  SDL_RWclose(file);
  binocle_log_info("%d bytes read of %d", nb_read_total, res_size);
  if (nb_read_total != res_size) {
    binocle_log_error("Size mismatch");
    SDL_free(res);
    return false;
  }

  res[nb_read_total] = '\0';
  *buffer = res;
  *buffer_length = res_size + 1;
  return true;
}

bool binocle_sdl_write_text_file(char *filename, char *buffer, size_t size) {
  binocle_log_info("Writing text file: %s", filename);
  SDL_RWops *file = SDL_RWFromFile(filename, "wb");
  if (file == NULL) {
    binocle_log_error("Cannot open text file");
    return false;
  }

  if (SDL_RWwrite(file, buffer, 1, size) != size) {
    binocle_log_error("Error writing to file");
    return false;
  }

  SDL_RWclose(file);
  return true;
}

bool binocle_sdl_load_binary_file(char *filename, char **buffer, size_t *buffer_length) {
  binocle_log_info("Loading binary file: %s", filename);
  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open binary file: %s Reason: %s", filename, SDL_GetError());
    return false;
  }

  Sint64 res_size = SDL_RWsize(file);
  if (res_size == 0) {
    binocle_log_error("Stream size is zero.");
    return false;
  }
  if (res_size < 0) {
    binocle_log_error("Error getting size of stream: %s", SDL_GetError());
    return false;
  }

  char *res = (char *) SDL_malloc(res_size);

  size_t nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = SDL_RWread(file, buf, 1, ((size_t)res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }
  SDL_RWclose(file);
  binocle_log_info("%d bytes read of %lld", nb_read_total, res_size);
  if (nb_read_total != res_size) {
    binocle_log_error("Size mismatch");
    SDL_free(res);
    return false;
  }

  *buffer = res;
  *buffer_length = res_size;
  return true;
}

char *binocle_sdl_assets_dir() {
  char *binocle_assets_dir = NULL;
#if defined(__EMSCRIPTEN__)
  binocle_assets_dir = malloc(1024);
  sprintf(binocle_assets_dir, "/assets/");
#elif defined(__WINDOWS__)
  char *base_path = SDL_GetBasePath();
  if (base_path) {
    binocle_assets_dir = malloc(strlen(base_path) + 7);
    sprintf(binocle_assets_dir, "%s%s", base_path, "assets\\");
  } else {
    binocle_assets_dir = SDL_strdup(".\\assets");
  }
#elif defined(__ANDROID__)
  binocle_assets_dir = SDL_strdup("");
#else
  char *base_path = SDL_GetBasePath();
  if (base_path) {
    binocle_assets_dir = base_path;
  } else {
    binocle_assets_dir = SDL_strdup("./");
  }
#endif
  return binocle_assets_dir;
}

bool binocle_sdl_filename_ends_with(const char *str, const char *suffix) {
  if (!str || !suffix) {
    return false;
  }
  size_t lenstr = SDL_strlen(str);
  size_t lensuffix = SDL_strlen(suffix);
  if (lensuffix >  lenstr) {
    return false;
  }
  return SDL_strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// You must free the result if result is non-NULL.
char *binocle_sdl_str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  size_t len_rep;  // length of rep (the string to remove)
  size_t len_with; // length of with (the string to replace rep with)
  size_t len_front; // distance between rep and end of last rep
  size_t count;    // number of replacements

  // sanity checks and initialization
  if (!orig || !rep)
    return NULL;
  len_rep = SDL_strlen(rep);
  if (len_rep == 0)
    return NULL; // empty rep causes infinite loop during count
  if (!with)
    with = "";
  len_with = SDL_strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; (tmp = SDL_strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = SDL_malloc(SDL_strlen(orig) + (len_with - len_rep) * count + 1);

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

bool binocle_sdl_file_exists(const char *filename) {
  SDL_RWops *f = SDL_RWFromFile(filename, "r");
  if (f == NULL) {
    return false;
  }
  SDL_RWclose(f);
  return true;
}

bool binocle_sdl_directory_exists(const char *path) {
  struct stat info;
  if( stat( path, &info ) != 0 )
    binocle_log_warning( "cannot access %s", path );
  else if( info.st_mode & S_IFDIR )
    return true;
  else
    binocle_log_warning( "%s is not a directory\n", path );
  return false;
}