//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <stdlib.h>
#include "binocle_app.h"
#include "binocle_sdl.h"

#define SOKOL_IMPL
#include <sokol_time.h>

binocle_app binocle_app_new() {
  binocle_app res = {0};
  res.assets_mount_path = SDL_strdup("/assets");
  return res;
}

bool binocle_app_init(binocle_app *app) {
  // Initialize time stuff
  stm_setup();

  // Initialize the filesystem
  app->fs = binocle_fs_new();
  binocle_fs_init(&app->fs);

  binocle_fs_mount(binocle_sdl_assets_dir(), app->assets_mount_path, false);

  binocle_sdl_init();
  return true;
}

void binocle_app_destroy(binocle_app *app) {
  binocle_fs_destroy(&app->fs);
  free(app->assets_mount_path);
  binocle_sdl_exit();
}