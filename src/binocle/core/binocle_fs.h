//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_FS_H
#define BINOCLE_FS_H

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <physfs.h>

typedef struct binocle_fs {
  uint64_t dummy;
} binocle_fs;

typedef struct binocle_fs_file {
  PHYSFS_File *file;
  char *filename;
} binocle_fs_file;

typedef enum binocle_fs_supported {
  BINOCLE_FS_SDL,
  BINOCLE_FS_PHYSFS,
} binocle_fs_supported;

binocle_fs binocle_fs_new();
bool binocle_fs_init(binocle_fs *fs);
void binocle_fs_destroy(binocle_fs *fs);
bool binocle_fs_mount(char *path, char *mount_point, bool prepend_to_search_path);
bool binocle_fs_get_last_modification_time(char *filename, uint64_t *modtime);
void binocle_fs_enumerate(char * path, PHYSFS_EnumerateCallback callback, void *user_data);
bool binocle_fs_is_directory(const char *filename);
binocle_fs_file binocle_fs_open_read(const char *filename);
void binocle_fs_close(binocle_fs_file file);
bool binocle_fs_read(binocle_fs_file file, void **buffer, size_t *size);
bool binocle_fs_load_binary_file(const char *filename, void **buffer, size_t *size);
bool binocle_fs_load_text_file(const char *filename, char **buffer, size_t *size);
/// \brief Gets the directory part of the filename+path given as input
/// \param filename the full path including the filename
/// \param path the path without the filename and with no trailing slash
/// \param length the length of the returned path
void binocle_fs_get_directory(const char *filename, char *path, int *length);

/// \brief Return the file name given a full path
/// \param full_path the full path
/// \param filename the filename extracted from the path
/// \param length the length of the filename;
void binocle_fs_get_filename(const char *full_path, char *filename, int *length);

/// \brief Returns the filename without the extension
/// \param myStr the full filename (it can include the path)
/// \param extSep the separator character, e.g. "."
/// \param pathSep the path separator, e.g. "/"
/// \param retStr the filename without extension (including path if provided)
void binocle_fs_path_without_extension(char* myStr, char extSep, char pathSep, char *retStr);

#endif //BINOCLE_FS_H
