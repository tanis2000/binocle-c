//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_WREN_H
#define BINOCLE_WREN_H

#include "wren/src/include/wren.h"

typedef struct binocle_wren_t {
  WrenConfiguration config;
  WrenVM* vm;
} binocle_wren_t;

binocle_wren_t *binocle_wren_new();
void binocle_wren_init(struct binocle_wren_t *wren);
void binocle_wren_log(WrenVM *vm, const char* text);
void binocle_wren_error(WrenVM *vm, WrenErrorType type,
                        const char* module,
                        int line,
                        const char* message);

#endif //BINOCLE_WREN_H
