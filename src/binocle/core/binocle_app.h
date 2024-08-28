//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_APP_H
#define BINOCLE_APP_H

#include <inttypes.h>
#include <stdbool.h>
#include "binocle_fs.h"

typedef struct binocle_app_desc_t {
  /// if set we force the assets virtual path to point to this one
  char *forced_asset_origin_path;
} binocle_app_desc_t;

typedef struct binocle_app {
  binocle_fs fs;
  char *assets_mount_path;
} binocle_app;

binocle_app binocle_app_new();
bool binocle_app_init(binocle_app *app, const binocle_app_desc_t *desc);
void binocle_app_destroy(binocle_app *app);

#endif //BINOCLE_APP_H
