//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_app.h"
#include "binocle_memory.h"
#include "binocle_sdl.h"
#include "sokol_gfx.h"
#include <stdlib.h>

#define SOKOL_IMPL
#include <sokol_time.h>

binocle_app binocle_app_new() {
  binocle_app res = {0};
  res.assets_mount_path = SDL_strdup("/assets");
  return res;
}

bool binocle_app_init(binocle_app *app, binocle_app_desc_t *desc) {
  // Initialize time stuff
  stm_setup();

  // Hide the console on Windows
#if defined(__WINDOWS__) && !defined(BINOCLE_SHOW_CONSOLE)
  const HWND windowHandle = GetConsoleWindow();
  ShowWindow(windowHandle, SW_HIDE);
#endif

  // Initialize the memory arena backend
  binocle_memory_init();

  // Initialize the filesystem
  app->fs = binocle_fs_new();
  binocle_fs_init(&app->fs);

  if (desc->forced_asset_origin_path != NULL) {
    binocle_fs_mount(desc->forced_asset_origin_path, app->assets_mount_path, false);
  } else {
    binocle_fs_mount(binocle_sdl_assets_dir(), app->assets_mount_path, false);
  }

  binocle_sdl_init();
  return true;
}

void binocle_app_destroy(binocle_app *app) {
  binocle_fs_destroy(&app->fs);
  SDL_free(app->assets_mount_path);
  binocle_sdl_exit();
}