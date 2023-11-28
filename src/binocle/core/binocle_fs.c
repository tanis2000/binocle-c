//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_fs.h"
#include "binocle_log.h"
#include <physfs.h>
#include <SDL.h>

#define CUTE_PATH_IMPLEMENTATION
#include <cute_path/cute_path.h>

binocle_fs binocle_fs_new() {
  binocle_fs res = {0};
  return res;
}

bool binocle_fs_init(binocle_fs *fs) {
  if (!PHYSFS_init(NULL)) {
    return false;
  }

  return true;
}

void binocle_fs_destroy(binocle_fs *fs) {
  PHYSFS_deinit();
}

bool binocle_fs_mount(char *path, char *mount_point, bool prepend_to_search_path) {
  if (!PHYSFS_mount(path, mount_point, prepend_to_search_path)) {
    binocle_log_error("Cannot mount %s at %s", path, mount_point);
    binocle_log_error("Error is %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  binocle_log_info("Mounted %s at %s", path, PHYSFS_getMountPoint(path));
  return true;
}

bool binocle_fs_get_last_modification_time(char *filename, uint64_t *modtime) {
  PHYSFS_Stat stat;
  if (!PHYSFS_stat(filename, &stat)) {
    binocle_log_error(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return false;
  }
  *modtime = stat.modtime;
  return true;
}

void binocle_fs_enumerate(char * path, PHYSFS_EnumerateCallback callback, void *user_data) {
  if (!PHYSFS_enumerate(path, callback, user_data)) {
    binocle_log_error(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
  }
}

bool binocle_fs_is_directory(const char *filename) {
  PHYSFS_Stat stat = {0};
  PHYSFS_stat(filename, &stat);
  return (stat.filetype == PHYSFS_FILETYPE_DIRECTORY);
}

binocle_fs_file binocle_fs_open_read(const char *filename) {
  binocle_fs_file res = {0};
  res.filename = SDL_malloc(SDL_strlen(filename) + 1);
  strcpy(res.filename, filename);
  res.file = PHYSFS_openRead(filename);
  if (res.file == NULL) {
    PHYSFS_ErrorCode error_code = PHYSFS_getLastErrorCode();
    binocle_log_error("Cannot open binary file: %s Reason: %s", filename, PHYSFS_getErrorByCode(error_code));
  }
  return res;
}

void binocle_fs_close(binocle_fs_file file) {
  if (file.file == NULL) {
    binocle_log_warning("Trying to close a file which has not been opened: %s", file.filename);
    SDL_free(file.filename);
    return;
  }
  SDL_free(file.filename);
  PHYSFS_close(file.file);
}

bool binocle_fs_read(binocle_fs_file file, void **buffer, size_t *buffer_length) {
  PHYSFS_sint64 res_size = PHYSFS_fileLength(file.file);
  if (res_size == 0) {
    binocle_log_error("File size is zero.");
    return false;
  }

  char *res = (char *) SDL_malloc(res_size);

  size_t nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = PHYSFS_readBytes(file.file, buf, ((size_t)res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }

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

bool binocle_fs_read_text(binocle_fs_file file, void **buffer, size_t *buffer_length) {
  PHYSFS_sint64 res_size = PHYSFS_fileLength(file.file);
  if (res_size == 0) {
    binocle_log_error("File size is zero.");
    return false;
  }

  char *res = (char *) SDL_malloc(res_size + 1);

  size_t nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = PHYSFS_readBytes(file.file, buf, ((size_t)res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }

  binocle_log_info("%d bytes read of %lld", nb_read_total, res_size);
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

bool binocle_fs_load_binary_file(const char *filename, void **buffer, size_t *size) {
  binocle_log_info("Loading binary file: %s", filename);
  binocle_fs_file file = binocle_fs_open_read(filename);
  if (file.file == NULL) {
    binocle_log_error("Cannot open file %s", filename);
    return false;
  }

  bool res = binocle_fs_read(file, buffer, size);
  if (!res) {
    binocle_log_error("Cannot read file %s", filename);
    return false;
  }

  binocle_fs_close(file);

  return true;
}

bool binocle_fs_load_text_file(const char *filename, char **buffer, size_t *size) {
  binocle_log_info("Loading text file: %s", filename);
  binocle_fs_file file = binocle_fs_open_read(filename);
  if (file.file == NULL) {
    binocle_log_error("Cannot open file %s", filename);
    return false;
  }

  bool res = binocle_fs_read_text(file, buffer, size);
  if (!res) {
    binocle_log_error("Cannot read file %s", filename);
    return false;
  }

  binocle_fs_close(file);

  return true;
}

void binocle_fs_get_directory(const char *filename, char *path, int *length) {
  *length = path_pop(filename, path, NULL);
}

void binocle_fs_path_without_extension(char* myStr, char extSep, char pathSep, char *retStr) {
  char *lastExt, *lastPath;

  // Error checks and allocate string.

  if (myStr == NULL) return;
  if (retStr == NULL) return;

  // Make a copy and find the relevant characters.

  strcpy (retStr, myStr);
  lastExt = strrchr (retStr, extSep);
  lastPath = (pathSep == 0) ? NULL : strrchr (retStr, pathSep);

  // If it has an extension separator.

  if (lastExt != NULL) {
    // and it's to the right of the path separator.

    if (lastPath != NULL) {
      if (lastPath < lastExt) {
        // then remove it.

        *lastExt = '\0';
      }
    } else {
      // Has extension separator with no path separator.

      *lastExt = '\0';
    }
  }
}

void binocle_fs_get_filename(const char *full_path, char *filename, int *length) {
  char path[CUTE_PATH_MAX_PATH];
  *length = path_pop(full_path, path, filename);
}