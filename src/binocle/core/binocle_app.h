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

typedef struct binocle_app {
  binocle_fs fs;
  char *assets_mount_path;
} binocle_app;

binocle_app binocle_app_new();
bool binocle_app_init(binocle_app *app);
void binocle_app_destroy(binocle_app *app);

#endif //BINOCLE_APP_H
