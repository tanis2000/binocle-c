//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_fs.h"
#include "binocle_log.h"
#include <physfs.h>

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
    binocle_log_error(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
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