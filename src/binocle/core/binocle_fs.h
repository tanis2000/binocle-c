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

binocle_fs binocle_fs_new();
bool binocle_fs_init(binocle_fs *fs);
void binocle_fs_destroy(binocle_fs *fs);
bool binocle_fs_mount(char *path, char *mount_point, bool prepend_to_search_path);
bool binocle_fs_get_last_modification_time(char *filename, uint64_t *modtime);
void binocle_fs_enumerate(char * path, PHYSFS_EnumerateCallback callback, void *user_data);

#endif //BINOCLE_FS_H
